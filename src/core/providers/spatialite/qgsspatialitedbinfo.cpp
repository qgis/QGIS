/***************************************************************************
     qgsspatialitedbinfo.cpp
     --------------------------------------
    Date                 : 2017-05-14
    Copyright         : (C) 2017 by Mark Johnson, Berlin Germany
    Email                : mj10777 at googlemail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QStringList>
#include <QRegularExpression>
#include <QDebug>
#include "qgssqlitehandle.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"

#include <gdal.h>
#include <cstring>
#include "qgslogger.h" // QgsDebugMsgLevel
#include "qgsmessagelog.h"
#include "qgsproject.h"
#include <qgsjsonutils.h>

//-----------------------------------------------------------------
// Class SpatialiteDbInfo
//-----------------------------------------------------------------
// SpatialiteDbInfo::SpatialiteDbInfo
//-----------------------------------------------------------------
const QString SpatialiteDbInfo::SPATIALITE_ARRAY_PREFIX = QStringLiteral( "json" );
const QString SpatialiteDbInfo::SPATIALITE_ARRAY_SUFFIX = QStringLiteral( "list" );
const QString SpatialiteDbInfo::ParseSeparatorGeneral = QStringLiteral( ";" );
const QString SpatialiteDbInfo::ParseSeparatorUris = QStringLiteral( ":" );
const QString SpatialiteDbInfo::ParseSeparatorCoverage = QStringLiteral( "€" );
QStringList SpatialiteDbInfo::mSpatialiteTypes;
SpatialiteDbInfo::SpatialiteDbInfo( QString sDatabaseFilename, sqlite3 *sqlite_handle, SpatialiteDbInfo::SpatialMetadata dbCreateOption )
  : mDatabaseFileName( sDatabaseFilename )
  , mFileName( QString::null )
  , mSqliteHandle( sqlite_handle )
  , mQSqliteHandle( nullptr )
  , mSpatialMetadata( SpatialiteDbInfo::SpatialUnknown )
  , mDbCreateOption( dbCreateOption )
  , mSpatialiteVersionInfo( QString::null )
  , mSpatialiteVersionMajor( -1 )
  , mSpatialiteVersionMinor( -1 )
  , mSpatialiteVersionRevision( -1 )
  , mRasterLite2VersionInfo( QString::null )
  , mRasterLite2VersionMajor( -1 )
  , mRasterLite2VersionMinor( -1 )
  , mRasterLite2VersionRevision( -1 )
  , mTotalCountLayers( 0 )
  , mHasVectorLayers( -1 )
  , mHasVectorLayersAuth( -1 )
  , mHasLegacyGeometryLayers( -1 )
  , mHasSpatialTables( 0 )
  , mHasSpatialViews( 0 )
  , mHasVirtualShapes( 0 )
  , mHasRasterLite1Tables( -1 )
  , mHasGdalRasterLite1Driver( false )
  , mHasVectorCoveragesTables( -1 )
  , mHasRasterCoveragesTables( -1 )
  , mHasGdalRasterLite2Driver( false )
  , mHasTopologyExportTables( -1 )
  , mHasMBTilesTables( -1 )
  , mHasGdalMBTilesDriver( false )
  , mHasGeoPackageTables( -1 )
  , mHasGeoPackageVectors( -1 )
  , mHasGeoPackageRasters( -1 )
  , mHasGdalGeoPackageDriver( false )
  , mHasFdoOgrTables( -1 )
  , mHasFdoOgrDriver( false )
  , mHasVectorStylesView( -1 )
  , mHasRasterStylesView( -1 )
  , mHasGcp( false )
  , mHasTopology( false )
  , mIsVersion45( false )
  , mLayersCount( 0 )
  , mReadOnly( false )
  , mLoadLayers( false )
  , mIsDbValid( false )
  , mIsDbLocked( false )
  , mIsEmpty( false )
  , mIsSpatialite( false )
  , mIsRasterLite2( false )
  , mIsSpatialite40( false )
  , mIsSpatialite45( false )
  , mIsGdalOgr( false )
  , mIsSqlite3( false )
  , mSniffType( SpatialiteDbInfo::SniffMinimal )
  , mMimeType( SpatialiteDbInfo::MimeUnknown )
{
  switch ( mDbCreateOption )
  {
    case SpatialMetadata::SpatialiteGpkg:
    case SpatialMetadata::SpatialiteMBTiles:
    case SpatialMetadata::Spatialite40:
    case SpatialMetadata::Spatialite45:
      break;
    case SpatialMetadata::SpatialiteLegacy:
      mDbCreateOption = SpatialMetadata::Spatialite40;
      break;
    case SpatialMetadata::SpatialUnknown:
    case SpatialMetadata::SpatialiteFdoOgr:
    default:
      mDbCreateOption = SpatialMetadata::SpatialUnknown;
      break;
  }
  QFileInfo file_info( mDatabaseFileName );
  // for canonicalFilePath, the file must exist
  mSpatialMetadataString = SpatialiteDbInfo::SpatialMetadataTypeName( mSpatialMetadata );
  mMimeType = SpatialiteDbInfo::readMagicHeaderFromFile( mDatabaseFileName );
  if ( mMimeType == SpatialiteDbInfo::MimeSqlite3 )
  {
    mDatabaseFileName = file_info.canonicalFilePath();
    mFileName = file_info.fileName();
    mDirectoryName = file_info.canonicalPath();
    // The File exists and is a Sqlite3 container
    mDbCreateOption = SpatialMetadata::SpatialUnknown;
    mIsSqlite3 = true;
    mIsDbValid = true;
  }
  else
  {
    // either the File does not exist or is not a Sqlite3 container
    if ( !file_info.exists() )
    {
      mMimeType = SpatialiteDbInfo::MimeNotExists;
      if ( mDbCreateOption != SpatialMetadata::SpatialUnknown )
      {
        // The File does not exist and the user desires a creation for a format that we support
        if ( createDatabase() )
        {
          mMimeType = SpatialiteDbInfo::MimeSqlite3;
          mIsDbValid = true;
          // The created Database exists and is valid and of the Container-Type requested
          // The canonicalFilePath is set during createDatabase when a valid sqlite3 has been determined
        }
      }
    }
  }
};
//-----------------------------------------------------------------
// SpatialiteDbInfo::~SpatialiteDbInfo
//-----------------------------------------------------------------
SpatialiteDbInfo::~SpatialiteDbInfo()
{
  for ( QMap<QString, SpatialiteDbLayer *>::iterator it = mDbLayers.begin(); it != mDbLayers.end(); ++it )
  {
    SpatialiteDbLayer *dbLayer = qobject_cast<SpatialiteDbLayer *>( it.value() );
    delete dbLayer;
  }
  mDbLayers.clear();
  if ( mQSqliteHandle )
  {
    // Tell QgsSqliteHandle that we are shutting down
    mQSqliteHandle->setSpatialiteDbInfo( nullptr );
    // closeDb will try to delete us if not nullptr
    QgsSqliteHandle::closeDb( mQSqliteHandle );
  }
  mSqliteHandle = nullptr;
  mQSqliteHandle = nullptr;
  mVectorLayers.clear();
  mVectorLayersTypes.clear();
  mRasterLite1Layers.clear();
  mVectorCoveragesLayers.clear();
  mRasterCoveragesLayers.clear();
  mVectorStyleData.clear();
  mVectorStyleInfo.clear();
  mRasterStyleData.clear();
  mRasterStyleInfo.clear();
  mLayerNamesStyles.clear();
  mTopologyNames.clear();
  mTopologyExportLayers.clear();
  mMBTilesLayers.clear();
  mGeoPackageLayers.clear();
  mFdoOgrLayers.clear();
  mSelectedLayersUris.clear();
  mNonSpatialTables.clear();
  mDbLayersDataSourceUris.clear();
  mErrors.clear();
  mWarnings.clear();
  mMapGroupNames.clear();
  mListGroupNames.clear();
};
//-----------------------------------------------------------------
// SpatialiteDbInfo::getSharedSpatialiteConnectionsCount
//-----------------------------------------------------------------
int SpatialiteDbInfo::getSharedSpatialiteConnectionsCount()
{
  if ( getQSqliteHandle() )
  {
    return getQSqliteHandle()->getSharedSpatialiteConnectionsCount();
  }
  return -1;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::getConnectionRef
//-----------------------------------------------------------------
// Returns the amount of connections to this Database Source
// -1 = not shared, otherwise starting with 1
//-----------------------------------------------------------------
int SpatialiteDbInfo::getConnectionRef() const
{
  if ( getQSqliteHandle() )
  {
    return getQSqliteHandle()->getRef();
  }
  return -1;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::removeConnectionRef
//-----------------------------------------------------------------
// Removes reference to this connection
// - Returns the amount of connections to this Database Source
//-----------------------------------------------------------------
int SpatialiteDbInfo::removeConnectionRef() const
{
  if ( getQSqliteHandle() )
  {
    return getQSqliteHandle()->removeRef();
  }
  return -1;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::checkConnectionNeeded
//-----------------------------------------------------------------
// Check if this is the last used connection
// - Inform QgsSqliteHandle that this connection is no longer being used
//-----------------------------------------------------------------
bool SpatialiteDbInfo::checkConnectionNeeded()
{
  if ( getQSqliteHandle() )
  {
    if ( getQSqliteHandle()->getRef() <= 1 )
    {
      // SpatialiteDbInfo can be deleted since it not being used elsewhere, Connection will be closed
      return false;
    }
    // Inform QgsSqliteHandle that this connection is no longer being used
    getQSqliteHandle()->removeRef();
    // SpatialiteDbInfo can be set to nullptr, but not deleted since it is being used elsewhere, Connection must remain open
    return true;
  }
  return false; // no connection, can be deleted
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::getConnectionUuid
//-----------------------------------------------------------------
// Returns unique id of this connection to this Database Source
// - set when SpatialiteDbInfo is set in QgsSqliteHandle
//-----------------------------------------------------------------
QString SpatialiteDbInfo::getConnectionUuid() const
{
  if ( getQSqliteHandle() )
  {
    return getQSqliteHandle()->getUuid();
  }
  return QString();
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::isConnectionShared
//-----------------------------------------------------------------
bool SpatialiteDbInfo::isConnectionShared() const
{
  if ( getQSqliteHandle() )
  {
    return getQSqliteHandle()->isShared();
  }
  return false;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::setConnectionShared
//-----------------------------------------------------------------
// In cases, such as in QgsDataItem *dataItem, sniffing is
//  done without sharing.
// When a valid Database for the Provider is found
//  the Connection will be set to shared, so that the
//  gathered Information must not be re-read
//-----------------------------------------------------------------
bool SpatialiteDbInfo::setConnectionShared( bool bIsShared )
{
  if ( getQSqliteHandle() )
  {
    return getQSqliteHandle()->setShared( bIsShared );
  }
  return false;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::isDbSpatialiteActive
//-----------------------------------------------------------------
// Used to test if Spatialite is active
//-----------------------------------------------------------------
bool SpatialiteDbInfo::isDbSpatialiteActive() const
{
  if ( getQSqliteHandle() )
  {
    return getQSqliteHandle()->isDbSpatialiteActive();
  }
  return false;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::isDbRasterLite2Active
//-----------------------------------------------------------------
// Used to test if RasterLite2 is active
//-----------------------------------------------------------------
bool SpatialiteDbInfo::isDbRasterLite2Active() const
{
  if ( getQSqliteHandle() )
  {
    return getQSqliteHandle()->isDbRasterLite2Active();
  }
  return false;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::dbHasSpatialite
//-----------------------------------------------------------------
// This should only be called when a Spatialite-Connection
// - should be started, if not active already
// Use isDbSpatialiteActive() to only test if active
//-----------------------------------------------------------------
bool SpatialiteDbInfo::dbHasSpatialite( bool loadSpatialite,  bool loadRasterLite2 )
{
  if ( ( getQSqliteHandle() ) && ( getQSqliteHandle()->isDbValid() ) )
  {
    if ( loadRasterLite2 )
    {
      return dbHasRasterlite2();
    }
    if ( loadSpatialite )
    {
      return getSpatialiteVersion();
    }
    return getQSqliteHandle()->isDbSpatialiteActive() ;
  }
  return false;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::dbHasRasterlite2
//-----------------------------------------------------------------
// This should only be called when a Spatialite/RasterLite2-Connection
// - should be started, if not active already
// Use isDbRasterLite2Active() to only test if active
//-----------------------------------------------------------------
bool SpatialiteDbInfo::dbHasRasterlite2()
{
  bool bRc = false;
  if ( ( getQSqliteHandle() ) && ( getQSqliteHandle()->isDbValid() ) )
  {
    // check if this has been called before
    if ( mRasterLite2VersionMajor < 0 )
    {
      if ( getQSqliteHandle()->initRasterlite2() )
      {
        if ( mSpatialiteVersionMajor < 0 )
        {
          getSpatialiteVersion();
        }
        // Will only run if QGis has been compiled with RasterLite2 support [RASTERLITE2_VERSION_GE_*]
        sqlite3_stmt *stmt = nullptr;
        QString sql = QStringLiteral( "SELECT RL2_Version()" );
        int i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
        if ( i_rc == SQLITE_OK )
        {
          while ( sqlite3_step( stmt ) == SQLITE_ROW )
          {
            if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
            {
              mRasterLite2VersionInfo = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
              QString sVersionInfo = mRasterLite2VersionInfo;
              sVersionInfo.remove( QRegExp( QString::fromUtf8( "[-`~!@#$%^&*()_—+=|:;<>«»,?/{a-zA-Z}\'\"\\[\\]\\\\]" ) ) );
              QStringList spatialiteParts = sVersionInfo.split( ' ', QString::SkipEmptyParts );
              // Get major, minor and revision version
              QStringList spatialiteVersionParts = spatialiteParts[0].split( '.', QString::SkipEmptyParts );
              if ( spatialiteVersionParts.size() == 3 )
              {
                mRasterLite2VersionMajor = spatialiteVersionParts[0].toInt();
                mRasterLite2VersionMinor = spatialiteVersionParts[1].toInt();
                mRasterLite2VersionRevision = spatialiteVersionParts[2].toInt();
              }
            }
          }
          sqlite3_finalize( stmt );
        }
      } // else: QGis may not have been compiled with RasterLite2 support [RASTERLITE2_VERSION_GE_*]
      else
      {
        QgsDebugMsgLevel( QString( "Connecting to: RasterLite2 failed. Spatialite[%1] RasterLite2[%2]" ).arg( isDbRasterLite2Active() ).arg( isDbRasterLite2Active() ), 4 );
      }
    }
    if ( mRasterLite2VersionMajor > 0 )
    {
      bRc = true;
    }
  }
  return bRc;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::FetchSpatialiteDbInfo
//-----------------------------------------------------------------
SpatialiteDbInfo *SpatialiteDbInfo::FetchSpatialiteDbInfo( const QString sDatabaseFileName, bool bShared, QString sLayerName, bool bLoadLayers, SpatialiteDbInfo::SpatialMetadata dbCreateOption,  SpatialSniff sniffType )
{
  bool bInitSpatialite = false;
  SpatialiteDbInfo *spatialiteDbInfo = new SpatialiteDbInfo( sDatabaseFileName, nullptr, dbCreateOption );
  sniffType = SpatialiteDbInfo::SniffMinimal;
  if ( bLoadLayers )
  {
    sniffType = SpatialiteDbInfo::SniffLoadLayers;
    bInitSpatialite = true;
  }
  if ( spatialiteDbInfo )
  {
    // The File exists and is a Sqlite3 container [will be created if a valid create-option is given and the file does not exist]
    if ( spatialiteDbInfo->isDbSqlite3() )
    {
      // The File exists  is a Sqlite3 container [will be created aboveif a valid create-option is given and the file does not exist]
      sqlite3 *sqlite_handle = nullptr;
      int open_flags = bShared ? SQLITE_OPEN_READWRITE : SQLITE_OPEN_READONLY | SQLITE_OPEN_NOMUTEX;
      // SpatialiteDbInfo will return the actual absolute path [with resolved soft-links]
      if ( QgsSqliteHandle::sqlite3_open_v2( spatialiteDbInfo->getDatabaseFileName().toUtf8().constData(), &sqlite_handle, open_flags, nullptr, bInitSpatialite ) == SQLITE_OK )
      {
        QgsSqliteHandle *handle = new QgsSqliteHandle( sqlite_handle, sDatabaseFileName, bShared, bInitSpatialite );
        if ( spatialiteDbInfo->attachQSqliteHandle( handle ) )
        {
          // Retrieve MetaData and all possible Layers if sLayerName is empty or Null
          if ( !spatialiteDbInfo->getSpatialiteLayerInfo( sLayerName, bLoadLayers, sniffType ) )
          {
            // For 'Drag and Drop' or 'Browser' logic:  Unknown (not supported) Sqlite3-Container (such as Fossil)
            if ( spatialiteDbInfo->isDbLocked() )
            {
              QgsDebugMsgLevel( QString( "The read Sqlite3 Container [%3,%5] is locked.. DbValid=%2 DbEmpty=%4 FileName[%1}" ).arg( sDatabaseFileName ).arg( spatialiteDbInfo->isDbValid() ).arg( spatialiteDbInfo->getFileMimeTypeString() ).arg( spatialiteDbInfo->isDbEmpty() ).arg( spatialiteDbInfo->dbSpatialMetadataString() ), 3 );
            }
            else
            {
              QgsDebugMsgLevel( QString( "The read Sqlite3 Container [%3,%5] is not supported by QgsSpatiaLiteProvider,QgsOgrProvider or QgsGdalProvider. DbValid=%2 DbEmpty=%4 FileName[%1}" ).arg( sDatabaseFileName ).arg( spatialiteDbInfo->isDbValid() ).arg( spatialiteDbInfo->getFileMimeTypeString() ).arg( spatialiteDbInfo->isDbEmpty() ).arg( spatialiteDbInfo->dbSpatialMetadataString() ), 3 );
            }
            // Allow this QgsSqliteHandle to be used for other purposes.
            // QgsSqliteHandle::openDb will return only the QgsSqliteHandle.
            // The Caller must close/delete it with QgsSqliteHandle::closeDb  manually
            return spatialiteDbInfo;
          }
          if ( spatialiteDbInfo->isDbGdalOgr() )
          {
            QgsDebugMsgLevel( QString( "Connection to the database was successful [for QgsOgrProvider or QgsGdalProvider only] (%1,%2,%3,%4) Layers-Loaded[%5] VectorLayers-Found[%6] dbPath[%7] SpatialiteConnections[%8] ConnectionRef[%10] DbEmpty=[%9] " ).arg( handle->getRef() ).arg( bShared ).arg( spatialiteDbInfo->isDbReadOnly() ).arg( spatialiteDbInfo->dbSpatialMetadataString() ).arg( spatialiteDbInfo->dbLoadedLayersCount() ).arg( spatialiteDbInfo->dbVectorLayersCount() ).arg( sDatabaseFileName ).arg( spatialiteDbInfo->getSharedSpatialiteConnectionsCount() ).arg( spatialiteDbInfo->isDbEmpty() ).arg( spatialiteDbInfo->getConnectionRef() ), 3 );
          }
          if ( spatialiteDbInfo->isDbSpatialite() )
          {
            QgsDebugMsgLevel( QString( "Connection to the database was successful [for QgsSpatiaLiteProvider] (%1,%2,%3,%4) Layers-Loaded[%5] VectorLayers-Found[%6] dbPath[%7] SpatialiteConnections[%8] ConnectionRef[%10] DbEmpty=[%9] " ).arg( handle->getRef() ).arg( bShared ).arg( spatialiteDbInfo->isDbReadOnly() ).arg( spatialiteDbInfo->dbSpatialMetadataString() ).arg( spatialiteDbInfo->dbLoadedLayersCount() ).arg( spatialiteDbInfo->dbVectorLayersCount() ).arg( sDatabaseFileName ).arg( spatialiteDbInfo->getSharedSpatialiteConnectionsCount() ).arg( spatialiteDbInfo->isDbEmpty() ).arg( spatialiteDbInfo->getConnectionRef() ), 3 );
          }
        }
        else
        {
          QgsDebugMsgLevel( QString( "Connection could not be attached to the SpatialiteDbInfo cause[%1]" ).arg( "sqlite3_db_readonly returned a invalid result" ), 4 );
          // Will close the Connection
          delete spatialiteDbInfo;
          spatialiteDbInfo = nullptr;
        }
      }
      else
      {
        // A Sqlite3 file could not be opened with the sqlite3 driver
        QgsDebugMsgLevel( QString( "Failure while connecting to FileMimeType[%2]: %1\n " ).arg( sDatabaseFileName ).arg( spatialiteDbInfo->getFileMimeTypeString() ), 4 );
      }
    }
    else
    {
      // For 'Drag and Drop' or 'Browser' logic: use another Provider to determine a non-Sqlite3 file
      if ( spatialiteDbInfo->getFileMimeType() == SpatialiteDbInfo::MimeNotExists )
      {
        // File does not exist
        delete spatialiteDbInfo;
        spatialiteDbInfo = nullptr;
        QgsDebugMsgLevel( QString( "File does not exist FileName[%1]" ).arg( sDatabaseFileName ), 7 );
      }
      else
      {
        if ( spatialiteDbInfo->getQSqliteHandle() )
        {
          spatialiteDbInfo->getQSqliteHandle()->invalidate(); // Not a Database
        }
        QgsDebugMsgLevel( QString( "File is not a Sqlite3 container. MimeType[%2] FileName[%1]" ).arg( sDatabaseFileName ).arg( spatialiteDbInfo->getFileMimeTypeString() ), 7 );
      }
    }
  }
  return spatialiteDbInfo;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::setSpatialMetadata
//-----------------------------------------------------------------
void SpatialiteDbInfo::setSpatialMetadata( int iSpatialMetadata )
{
  mSpatialMetadata = ( SpatialiteDbInfo::SpatialMetadata )iSpatialMetadata;
  if ( mSpatialMetadata == SpatialMetadata::SpatialUnknown )
  {
    if ( ( mHasMBTilesTables == 0 ) && ( checkMBTiles() ) )
    {
      mSpatialMetadata = SpatialMetadata::SpatialiteMBTiles;
    }
  }
  mSpatialMetadataString = SpatialiteDbInfo::SpatialMetadataTypeName( mSpatialMetadata );
  switch ( mSpatialMetadata )
  {
    case SpatialMetadata::SpatialiteFdoOgr:
      mIsDbValid = true;
      mIsGdalOgr = true;
      break;
    case SpatialMetadata::SpatialiteGpkg:
    case SpatialMetadata::SpatialiteMBTiles:
      mIsDbValid = true;
      mIsGdalOgr = true;
      break;
    case SpatialMetadata::SpatialiteLegacy:
      mIsSpatialite = true;
      mIsDbValid = true;
      break;
    case SpatialMetadata::Spatialite40:
      mIsSpatialite = true;
      mIsSpatialite40 = true;
      mIsDbValid = true;
      break;
    case SpatialMetadata::Spatialite45:
      mIsSpatialite = true;
      mIsSpatialite40 = true;
      mIsSpatialite45 = true;
      mIsDbValid = true;
      break;
    case SpatialMetadata::SpatialUnknown:
    default:
      mIsDbValid = false;
      break;
  }
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::attachQSqliteHandle
//-----------------------------------------------------------------
bool SpatialiteDbInfo::attachQSqliteHandle( QgsSqliteHandle *qSqliteHandle )
{
  if ( ( qSqliteHandle ) && ( qSqliteHandle->handle() ) )
  {
    // If the Database was created, then there may be a mismatch between the canonicalFilePath and the given file-name
    QFileInfo file_info( qSqliteHandle->dbPath() );
    if ( mDatabaseFileName == file_info.canonicalFilePath() )
    {
      mQSqliteHandle = qSqliteHandle;
      if ( mQSqliteHandle->getRef() == 0 )
      {
        setConnectionShared( true );
      }
      if ( !mSqliteHandle )
      {
        mSqliteHandle = mQSqliteHandle->handle();
      }
      else
      {
        if ( mSqliteHandle != mQSqliteHandle->handle() )
        {
          int i_rc = 0;
          // Note: travis reports: error: use of undeclared identifier 'sqlite3_db_readonly'
          i_rc = sqlite3_db_readonly( mQSqliteHandle->handle(), "main" );
          switch ( i_rc )
          {
            case 0:
              mReadOnly = false;
              break;
            case 1:
              mReadOnly = true;
              break;
            case -1:
            default:
              setDatabaseInvalid( qSqliteHandle->dbPath(), QString( "attachQSqliteHandle: sqlite3_db_readonly failed." ) );
              mIsSqlite3 = false;
              break;
          }
          if ( isDbSqlite3() )
          {
            mSqliteHandle = mQSqliteHandle->handle();
          }
        }
      }
    }
    else
    {
      setDatabaseInvalid( qSqliteHandle->dbPath(), QString( "attachQSqliteHandle: Database File mismatch." ) );
    }
  }
  else
  {
    setDatabaseInvalid( qSqliteHandle->dbPath(), QString( "attachQSqliteHandle: other errors." ) );
  }
  if ( ( mQSqliteHandle ) && ( isDbSqlite3() ) )
  {
    // The used canonicalFilePath will be set and used in QSqliteHandle
    mQSqliteHandle->setSpatialiteDbInfo( ( SpatialiteDbInfo * )this );
  }
  return isDbSqlite3();
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::GetVariantType
//-----------------------------------------------------------------
SpatialiteDbInfo::TypeSubType SpatialiteDbInfo::GetVariantType( const QString &type )
{
  // making some assumptions in order to guess a more realistic type
  if ( type == QLatin1String( "int" ) ||
       type == QLatin1String( "integer" ) ||
       type == QLatin1String( "integer64" ) ||
       type == QLatin1String( "bigint" ) ||
       type == QLatin1String( "smallint" ) ||
       type == QLatin1String( "tinyint" ) ||
       type == QLatin1String( "boolean" ) )
    return TypeSubType( QVariant::LongLong, QVariant::Invalid );
  else if ( type == QLatin1String( "real" ) ||
            type == QLatin1String( "double" ) ||
            type == QLatin1String( "double precision" ) ||
            type == QLatin1String( "float" ) )
    return TypeSubType( QVariant::Double, QVariant::Invalid );
  else if ( type == QLatin1String( "date" ) )
    return TypeSubType( QVariant::Date, QVariant::Invalid );
  else if ( type == QLatin1String( "datetime" ) )
    return TypeSubType( QVariant::DateTime, QVariant::Invalid );
  else if ( type.startsWith( SpatialiteDbInfo::SPATIALITE_ARRAY_PREFIX ) && type.endsWith( SpatialiteDbInfo::SPATIALITE_ARRAY_SUFFIX ) )
  {
    // New versions of OGR convert list types (StringList, IntegerList, Integer64List and RealList)
    // to JSON when it stores a Spatialite table. It sets the column type as JSONSTRINGLIST,
    // JSONINTEGERLIST, JSONINTEGER64LIST or JSONREALLIST
    TypeSubType subType = SpatialiteDbInfo::GetVariantType( type.mid( SpatialiteDbInfo::SPATIALITE_ARRAY_PREFIX.size(),
                          type.size() - SpatialiteDbInfo::SPATIALITE_ARRAY_PREFIX.size() - SpatialiteDbInfo::SPATIALITE_ARRAY_SUFFIX.size() ) );
    return TypeSubType( subType.first == QVariant::String ? QVariant::StringList : QVariant::List, subType.first );
  }
  else
    // for sure any SQLite value can be represented as SQLITE_TEXT
    return TypeSubType( QVariant::String, QVariant::Invalid );
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::getSpatialiteTypes
//-----------------------------------------------------------------
QStringList SpatialiteDbInfo::getSpatialiteTypes()
{
  if ( mSpatialiteTypes.count() == 0 )
  {
    mSpatialiteTypes.append( "SpatialIndex-Data" );
    mSpatialiteTypes.append( "Spatialite-Internal" );
    mSpatialiteTypes.append( "Spatialite-Virtual" );
    mSpatialiteTypes.append( "Spatialite2-Internal" );
    mSpatialiteTypes.append( "Geometries-Internal" );
    mSpatialiteTypes.append( "SpatialTable-Data" );
    mSpatialiteTypes.append( "SpatialTable-Internal" );
    mSpatialiteTypes.append( "SpatialView-Data" );
    mSpatialiteTypes.append( "SpatialView-Internal" );
    mSpatialiteTypes.append( "VirtualShapes-Data" );
    mSpatialiteTypes.append( "VirtualShapes-Internal" );
    mSpatialiteTypes.append( "RasterLite1-Internal" );
    mSpatialiteTypes.append( "RasterLite1-Metadata" );
    mSpatialiteTypes.append( "RasterLite1-Raster" );
    mSpatialiteTypes.append( "RasterLite2-Internal" );
    mSpatialiteTypes.append( "Styles-Internal" );
    mSpatialiteTypes.append( "Topology-Internal" );
    mSpatialiteTypes.append( "Sqlite3-Internal" );
    mSpatialiteTypes.append( "Trigger-Data" );
    mSpatialiteTypes.append( "EPSG-Table" );
    mSpatialiteTypes.append( "EPSG-Internal" );
    mSpatialiteTypes.append( "Table-Data" );
    mSpatialiteTypes.append( "View-Data" );
    mSpatialiteTypes.append( "GeoPackage-Raster-Data" );
    mSpatialiteTypes.append( "GeoPackage-Vector-Data" );
    mSpatialiteTypes.append( "GeoPackage-Internal" );
    mSpatialiteTypes.append( "MBTiles-Raster" );
    mSpatialiteTypes.append( "MBTiles-Index" );
    mSpatialiteTypes.append( "MBTiles-Internal" );
  }
  return mSpatialiteTypes;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo enum functions
// - converting enum to string/name or QIcon
//-----------------------------------------------------------------
// SpatialiteDbInfo::NonSpatialTablesTypeIcon
//-----------------------------------------------------------------
QIcon SpatialiteDbInfo::NonSpatialTablesTypeIcon( QString typeName )
{
  if ( typeName.startsWith( QLatin1String( "Table" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddTable.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "View" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mActionCalculateField.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "SpatialTable" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mActionNewVectorLayer.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "SpatialView" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mActionNodeTool.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "VirtualShapes" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/rendererPointDisplacementSymbol.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "Virtual" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/rendererGraduatedSymbol.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "SpatialIndex" ), Qt::CaseInsensitive ) )
  {
    if ( typeName.endsWith( QLatin1String( "-Rowid" ), Qt::CaseInsensitive ) )
    {
      return QgsApplication::getThemeIcon( QStringLiteral( "/mActionFormAnnotation.svg" ) );
    }
    else if ( typeName.endsWith( QLatin1String( "-Parent" ), Qt::CaseInsensitive ) )
    {
      return QgsApplication::getThemeIcon( QStringLiteral( "/mActionPropertyItem.svg" ) );
    }
    else if ( typeName.endsWith( QLatin1String( "-Node" ), Qt::CaseInsensitive ) )
    {
      return QgsApplication::getThemeIcon( QStringLiteral( "/mActionMergeFeatureAttributes.svg" ) );
    }
    return QgsApplication::getThemeIcon( QStringLiteral( "/mActionFilterTableFields.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "Geometries" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddWfsLayer.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "RasterLite2Raster" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mIconColorSwatches.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "RasterLite2" ), Qt::CaseInsensitive ) )
  {
    if ( typeName.endsWith( QLatin1String( "-Admin" ), Qt::CaseInsensitive ) )
    {
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconColorSwatches.svg" ) );
    }
    else if ( typeName.endsWith( QLatin1String( "-Metadata" ), Qt::CaseInsensitive ) )
    {
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconColorSwatches.svg" ) );
    }
    else if ( typeName.endsWith( QLatin1String( "-Tiles" ), Qt::CaseInsensitive ) )
    {
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconRasterGroup.svg" ) );
    }
    return QgsApplication::getThemeIcon( QStringLiteral( "/mIconColorSwatches.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "RasterLite1-Metadata" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mActionNewAttribute.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "RasterLite1-Tiles" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/rendererGraduatedSymbol.svg" ) );
  }
  else if ( ( typeName.startsWith( QLatin1String( "RasterLite1" ), Qt::CaseInsensitive ) ) ||
            ( typeName.endsWith( QLatin1String( "RasterLite1" ), Qt::CaseInsensitive ) ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mIconColorBox.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "MBTiles" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mIconRasterGroup.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "GeoPackage-Tiles" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mIconRasterGroup.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "GeoPackage-Vector" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mActionNewVectorLayer.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "GeoPackage-Admin" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/providerGdal.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "GeoPackage" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/providerGdal.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "EPSG" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mIconProjectionEnabled.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "Trigger" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mActionMergeFeatureAttributes.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "Sqlite3" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mActionDataSourceManager.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "Topology" ), Qt::CaseInsensitive ) )
  {
    if ( typeName.startsWith( QLatin1String( "Topology-View" ), Qt::CaseInsensitive ) )
    {
      if ( typeName.endsWith( QLatin1String( "Edges_Seeds" ), Qt::CaseInsensitive ) )
      {
        return QgsApplication::getThemeIcon( QStringLiteral( "/mActionFormView.svg" ) );
      }
      else if ( typeName.endsWith( QLatin1String( "Face_Seeds" ), Qt::CaseInsensitive ) )
      {
        return QgsApplication::getThemeIcon( QStringLiteral( "/mActionFormView.svg" ) );
      }
      return QgsApplication::getThemeIcon( QStringLiteral( "/mActionFilterTableFields.svg" ) );
    }
    else
    {
      if ( typeName.endsWith( QLatin1String( "Face" ), Qt::CaseInsensitive ) )
      {
        return QgsApplication::getThemeIcon( QStringLiteral( "/mActionFormView.svg" ) );
      }
      else if ( typeName.endsWith( QLatin1String( "Edge" ), Qt::CaseInsensitive ) )
      {
        return QgsApplication::getThemeIcon( QStringLiteral( "/mActionFormView.svg" ) );
      }
      else if ( typeName.endsWith( QLatin1String( "Node" ), Qt::CaseInsensitive ) )
      {
        return QgsApplication::getThemeIcon( QStringLiteral( "/mActionMergeFeatureAttributes.svg" ) );
      }
      // Seed
      return QgsApplication::getThemeIcon( QStringLiteral( "/mActionMeasure.svg" ) );
    }
    return QgsApplication::getThemeIcon( QStringLiteral( "/rendererPointClusterSymbol.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "Styles" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/rendererRuleBasedSymbol.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "Spatialite2" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddLegend.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "Spatialite" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddSpatiaLiteLayer.svg" ) );
  }
  else
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/multieditChangedValues.svg" ) );
  }
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::SniffTypeString
//-----------------------------------------------------------------
QString SpatialiteDbInfo::SniffTypeString( SpatialiteDbInfo::SpatialSniff sniffType )
{
  //---------------------------------------------------------------
  switch ( sniffType )
  {
    case SpatialiteDbInfo::SniffDatabaseType:
      return QStringLiteral( "SniffDatabaseType" );
    case SpatialiteDbInfo::SniffMinimal:
      return QStringLiteral( "SniffMinimal" );
    case SpatialiteDbInfo::SniffLoadLayers:
      return QStringLiteral( "SniffLoadLayers" );
    case SpatialiteDbInfo::SniffExtendend:
      return QStringLiteral( "SniffExtendend" );
    default:
      return QStringLiteral( "SniffUnknown" );
  }
  //---------------------------------------------------------------
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::FileMimeTypeString
//-----------------------------------------------------------------
QString SpatialiteDbInfo::FileMimeTypeString( SpatialiteDbInfo::MimeType mimeType )
{
  //---------------------------------------------------------------
  switch ( mimeType )
  {
    case SpatialiteDbInfo::MimeSqlite3:
      return QStringLiteral( "application/sqlite3" );
    case SpatialiteDbInfo::MimeSqlite2:
      return QStringLiteral( "application/sqlite2" );
    case SpatialiteDbInfo::MimePdf:
      return QStringLiteral( "application/pdf" );
    case SpatialiteDbInfo::MimeGif87a:
      return QStringLiteral( "image/gif87a" );
    case SpatialiteDbInfo::MimeGif89a:
      return QStringLiteral( "image/gif89a" );
    case SpatialiteDbInfo::MimeTiff:
      return QStringLiteral( "image/tiff" );
    case SpatialiteDbInfo::MimeJpeg:
      return QStringLiteral( "image/jpeg" );
    case SpatialiteDbInfo::MimeJp2:
      return QStringLiteral( "image/jp2" );
    case SpatialiteDbInfo::MimePng:
      return QStringLiteral( "image/png" );
    case SpatialiteDbInfo::MimeIco:
      return QStringLiteral( "image/ico" );
    case SpatialiteDbInfo::MimeWebp :
      return QStringLiteral( "image/webp" );
    case SpatialiteDbInfo::MimeAvi :
      return QStringLiteral( "video/avi" );
    case SpatialiteDbInfo::MimeWav :
      return QStringLiteral( "audio/wav" );
    case SpatialiteDbInfo::MimeKmz:
      return QStringLiteral( "application/zip+kmz" );
    case SpatialiteDbInfo::MimeKml:
      return QStringLiteral( "application/xml+kml" );
    case SpatialiteDbInfo::MimeSvg:
      return QStringLiteral( "application/xml+svg" );
    case SpatialiteDbInfo::MimeScripts:
      return QStringLiteral( "application/scripts" );
    case SpatialiteDbInfo::MimeBash:
      return QStringLiteral( "application/bash" );
    case SpatialiteDbInfo::MimePython:
      return QStringLiteral( "application/python" );
    case SpatialiteDbInfo::MimeExeUnix:
      return QStringLiteral( "application/binary+unix" );
    case SpatialiteDbInfo::MimeHtmlDTD:
      return QStringLiteral( "application/html+dtd" );
    case SpatialiteDbInfo::MimeQGisProject:
      return QStringLiteral( "application/html+qgis" );
    case SpatialiteDbInfo::MimeTxt:
      return QStringLiteral( "text/plain" );
    case SpatialiteDbInfo::MimeRtf:
      return QStringLiteral( "text/rtf" );
    case SpatialiteDbInfo::MimePid:
      return QStringLiteral( "text/plain+pid" );
    case SpatialiteDbInfo::MimeZip:
      return QStringLiteral( "application/zip" );
    case SpatialiteDbInfo::MimeBz2:
      return QStringLiteral( "application/bz2" );
    case SpatialiteDbInfo::MimeTar:
      return QStringLiteral( "application/tar" );
    case SpatialiteDbInfo::MimeRar:
      return QStringLiteral( "application/rar" );
    case SpatialiteDbInfo::MimeXar:
      return QStringLiteral( "application/xar" );
    case SpatialiteDbInfo::Mime7z:
      return QStringLiteral( "application/7z" );
    case SpatialiteDbInfo::MimeXml:
      return QStringLiteral( "application/xml" );
    case SpatialiteDbInfo::MimeGdalPam:
      return QStringLiteral( "application/gdal+pam" );
    case SpatialiteDbInfo::MimeGdalGml:
      return QStringLiteral( "application/gdal+gml" );
    case SpatialiteDbInfo::MimeDirectoryNotReadable:
      return QStringLiteral( "MimeDirectoryNotReadable" );
    case SpatialiteDbInfo::MimeDirectory:
      return QStringLiteral( "MimeDirectory" );
    case SpatialiteDbInfo::MimeDirectorySymLink:
      return QStringLiteral( "MimeDirectorySymLink" );
    case SpatialiteDbInfo::MimeDirectoryExecutable:
      return QStringLiteral( "MimeDirectoryExecutable" );
    case SpatialiteDbInfo::MimeFileExecutable:
      return QStringLiteral( "MimeFileExecutable" );
    case SpatialiteDbInfo::MimeFileNotReadable:
      return QStringLiteral( "MimeFileNotReadable" );
    case SpatialiteDbInfo::MimeFile:
      return QStringLiteral( "MimeFile" );
    case SpatialiteDbInfo::MimeNotExists:
      return QStringLiteral( "MimeNotExists" );
    case SpatialiteDbInfo::MimeUnknown:
    default:
      return QStringLiteral( "MimeUnknown[%1]" ).arg( mimeType );
  }
  //---------------------------------------------------------------
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::SpatialiteLayerTypeName
//-----------------------------------------------------------------
QString SpatialiteDbInfo::SpatialiteLayerTypeName( SpatialiteDbInfo::SpatialiteLayerType layerType )
{
  //---------------------------------------------------------------
  switch ( layerType )
  {
    case SpatialiteDbInfo::SpatialTable:
      return QStringLiteral( "SpatialTable" );
    case SpatialiteDbInfo::SpatialView:
      return QStringLiteral( "SpatialView" );
    case SpatialiteDbInfo::VirtualShape:
      return QStringLiteral( "VirtualShape" );
    case SpatialiteDbInfo::RasterLite1:
      return QStringLiteral( "RasterLite1" );
    case SpatialiteDbInfo::RasterLite2Vector:
      return QStringLiteral( "RasterLite2Vector" );
    case SpatialiteDbInfo::RasterLite2Raster:
      return QStringLiteral( "RasterLite2Raster" );
    case SpatialiteDbInfo::SpatialiteTopology:
      return QStringLiteral( "SpatialiteTopology" );
    case SpatialiteDbInfo::TopologyExport:
      return QStringLiteral( "TopologyExport" );
    case SpatialiteDbInfo::StyleVector:
      return QStringLiteral( "StyleVector" );
    case SpatialiteDbInfo::StyleRaster:
      return QStringLiteral( "StyleRaster" );
    case SpatialiteDbInfo::GdalFdoOgr:
      return QStringLiteral( "GdalFdoOgr" );
    case SpatialiteDbInfo::GeoPackageVector:
      return QStringLiteral( "GeoPackageVector" );
    case SpatialiteDbInfo::GeoPackageRaster:
      return QStringLiteral( "GeoPackageRaster" );
    case SpatialiteDbInfo::MBTilesTable:
      return QStringLiteral( "MBTilesTable" );
    case SpatialiteDbInfo::MBTilesView:
      return QStringLiteral( "MBTilesView" );
    case SpatialiteDbInfo::Metadata:
      return QStringLiteral( "Metadata" );
    case SpatialiteDbInfo::AllSpatialLayers:
      return QStringLiteral( "AllSpatialLayers" );
    case SpatialiteDbInfo::NonSpatialTables:
      return QStringLiteral( "NonSpatialTables" );
    case SpatialiteDbInfo::AllLayers:
      return QStringLiteral( "AllLayers" );
    case SpatialiteDbInfo::SpatialiteUnknown:
      return QStringLiteral( "SpatialiteUnknown" );
    default:
      return QStringLiteral( "SpatialiteUnknown[%1]" ).arg( layerType );
  }
  //---------------------------------------------------------------
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::SpatialiteLayerTypeIcon
//-----------------------------------------------------------------
QIcon SpatialiteDbInfo::SpatialiteLayerTypeIcon( SpatialiteDbInfo::SpatialiteLayerType layerType )
{
  switch ( layerType )
  {
    case SpatialiteDbInfo::AllSpatialLayers:
    case SpatialiteDbInfo::SpatialTable:
    case SpatialiteDbInfo::VirtualShape:
    case SpatialiteDbInfo::RasterLite2Vector:
    case SpatialiteDbInfo::GdalFdoOgr:
    case SpatialiteDbInfo::GeoPackageVector:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mActionNewVectorLayer.svg" ) );
      break;
    case SpatialiteDbInfo::SpatialView:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mActionNodeTool.svg" ) );
      break;
    case SpatialiteDbInfo::RasterLite1:
    case SpatialiteDbInfo::RasterLite2Raster:
    case SpatialiteDbInfo::GeoPackageRaster:
    case SpatialiteDbInfo::MBTilesTable:
    case SpatialiteDbInfo::MBTilesView:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddMap.svg" ) );
      break;
      return QgsApplication::getThemeIcon( QStringLiteral( "/rendererPointClusterSymbol.svg" ) );
      break;
    case SpatialiteDbInfo::TopologyExport:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconTopologicalEditing.svg" ) );
      break;
    case SpatialiteDbInfo::StyleVector:
      return QgsApplication::getThemeIcon( QStringLiteral( "/rendererRuleBasedSymbol.svg" ) );
      break;
    case SpatialiteDbInfo::StyleRaster:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconColorSwatches.svg" ) );
      break;
    case SpatialiteDbInfo::NonSpatialTables:
    case SpatialiteDbInfo::AllLayers:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconListView.png" ) );
      break;
    default:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconRasterGroup.svg" ) );
      break;
  }
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::SpatialMetadataTypeIcon
//-----------------------------------------------------------------
QIcon SpatialiteDbInfo::SpatialMetadataTypeIcon( SpatialiteDbInfo::SpatialMetadata spatialMetadata )
{
  switch ( spatialMetadata )
  {
    case SpatialiteDbInfo::SpatialiteLegacy:
    case SpatialiteDbInfo::Spatialite40:
    case SpatialiteDbInfo::Spatialite45:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddSpatiaLiteLayer.svg" ) );
      break;
    case SpatialiteDbInfo::SpatialiteFdoOgr:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddOgrLayer.svg" ) );
      break;
    case SpatialiteDbInfo::SpatialiteGpkg:
    case SpatialiteDbInfo::SpatialiteMBTiles:
      return QgsApplication::getThemeIcon( QStringLiteral( "/providerGdal.svg" ) );
      break;
    default:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconRasterGroup.svg" ) );
      break;
  }
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::SpatialGeometryTypeIcon
//-----------------------------------------------------------------
QIcon SpatialiteDbInfo::SpatialGeometryTypeIcon( QgsWkbTypes::Type geometryType )
{
  switch ( geometryType )
  {
    case QgsWkbTypes::Point:
    case QgsWkbTypes::Point25D:
    case QgsWkbTypes::MultiPoint:
    case QgsWkbTypes::MultiPoint25D:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddNodesItem.svg" ) );
      break;
    case QgsWkbTypes::LineString:
    case QgsWkbTypes::LineString25D:
    case QgsWkbTypes::MultiLineString:
    case QgsWkbTypes::MultiLineString25D:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddPolyline.svg" ) );
      break;
    case QgsWkbTypes::Polygon:
    case QgsWkbTypes::Polygon25D:
    case QgsWkbTypes::MultiPolygon:
    case QgsWkbTypes::MultiPolygon25D:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddPolygon.svg" ) );
      break;
    default:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconRasterGroup.svg" ) );
      break;
  }
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::SpatialIndexTypeName
//-----------------------------------------------------------------
QString SpatialiteDbInfo::SpatialIndexTypeName( SpatialiteDbInfo::SpatialIndexType spatialIndexType )
{
  //---------------------------------------------------------------
  switch ( spatialIndexType )
  {
    case SpatialiteDbInfo::SpatialIndexRTree:
      return QStringLiteral( "SpatialIndexRTree" );
    case SpatialiteDbInfo::SpatialIndexMbrCache:
      return QStringLiteral( "SpatialIndexMbrCache" );
    case SpatialiteDbInfo::SpatialIndexNone:
    default:
      return QStringLiteral( "SpatialIndexNone" );
  }
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::SpatiaIndexTypeFromName
//-----------------------------------------------------------------
SpatialiteDbInfo::SpatialIndexType SpatialiteDbInfo::SpatiaIndexTypeFromName( const QString &typeName )
{
  //---------------------------------------------------------------
  if ( QString::compare( typeName, "SpatialIndexRTree", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::SpatialIndexRTree;
  }
  else if ( QString::compare( typeName, "SpatialIndexMbrCache", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::SpatialIndexMbrCache;
  }
  else
  {
    return SpatialiteDbInfo::SpatialIndexNone;
  }
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::SpatialMetadataTypeName
//-----------------------------------------------------------------
QString SpatialiteDbInfo::SpatialMetadataTypeName( SpatialiteDbInfo::SpatialMetadata spatialMetadataType )
{
  //---------------------------------------------------------------
  switch ( spatialMetadataType )
  {
    case SpatialiteDbInfo::SpatialiteLegacy:
      return QStringLiteral( "SpatialiteLegacy" );
    case SpatialiteDbInfo::SpatialiteFdoOgr:
      return QStringLiteral( "SpatialiteFdoOgr" );
    case SpatialiteDbInfo::Spatialite40:
      return QStringLiteral( "Spatialite40" );
    case SpatialiteDbInfo::SpatialiteGpkg:
      return QStringLiteral( "SpatialiteGpkg" );
    case SpatialiteDbInfo::Spatialite45:
      return QStringLiteral( "Spatialite45" );
    case SpatialiteDbInfo::SpatialiteMBTiles:
      return QStringLiteral( "SpatialiteMBTiles" );
    case SpatialiteDbInfo::SpatialUnknown:
    default:
      return QStringLiteral( "SpatialUnknown" );
  }
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::SpatialMetadataTypeFromName
//-----------------------------------------------------------------
SpatialiteDbInfo::SpatialMetadata SpatialiteDbInfo::SpatialMetadataTypeFromName( const QString &typeName )
{
  //---------------------------------------------------------------
  if ( QString::compare( typeName, "SpatialiteLegacyr", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::SpatialiteLegacy;
  }
  else if ( QString::compare( typeName, "SpatialiteFdoOgr", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::SpatialiteFdoOgr;
  }
  else if ( QString::compare( typeName, "Spatialite40", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::Spatialite40;
  }
  else if ( QString::compare( typeName, "SpatialiteGpkg", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::SpatialiteGpkg;
  }
  else if ( QString::compare( typeName, "Spatialite45", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::Spatialite45;
  }
  else if ( QString::compare( typeName, "SpatialiteMBTiles", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::SpatialiteMBTiles;
  }
  else
  {
    return SpatialiteDbInfo::SpatialUnknown;
  }
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::SpatialiteLayerTypeFromName
//-----------------------------------------------------------------
SpatialiteDbInfo::SpatialiteLayerType SpatialiteDbInfo::SpatialiteLayerTypeFromName( const QString &typeName )
{
  //---------------------------------------------------------------
  if ( QString::compare( typeName, "SpatialTable", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::SpatialTable;
  }
  else if ( QString::compare( typeName, "SpatialView", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::SpatialView;
  }
  else if ( QString::compare( typeName, "VirtualShape", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::VirtualShape;
  }
  else if ( QString::compare( typeName, "RasterLite1", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::RasterLite1;
  }
  else if ( QString::compare( typeName, "RasterLite2Vector", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::RasterLite2Vector;
  }
  else if ( QString::compare( typeName, "RasterLite2Raster", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::RasterLite2Raster;
  }
  else if ( QString::compare( typeName, "SpatialiteTopology", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::SpatialiteTopology;
  }
  else if ( QString::compare( typeName, "TopologyExport", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::TopologyExport;
  }
  else if ( QString::compare( typeName, "StyleVector", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::StyleVector;
  }
  else if ( QString::compare( typeName, "StyleRaster", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::StyleRaster;
  }
  else if ( QString::compare( typeName, "GdalFdoOgr", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::GdalFdoOgr;
  }
  else if ( QString::compare( typeName, "GeoPackageVector", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::GeoPackageVector;
  }
  else if ( QString::compare( typeName, "GeoPackageRaster", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::GeoPackageRaster;
  }
  else if ( QString::compare( typeName, "MBTilesTable", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::MBTilesTable;
  }
  else if ( QString::compare( typeName, "MBTilesView", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::MBTilesView;
  }
  else if ( QString::compare( typeName, "NonSpatialTables", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::NonSpatialTables;
  }
  else if ( QString::compare( typeName, "Metadata", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::Metadata;
  }
  else if ( QString::compare( typeName, "AllSpatialLayers", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::AllSpatialLayers;
  }
  else
  {
    return SpatialiteDbInfo::SpatialiteUnknown;
  }
  //---------------------------------------------------------------
}
//-----------------------------------------------------------------
// SpatialiteDbInfo General functions:
//-----------------------------------------------------------------
// SpatialiteDbInfo::setDatabaseInvalid
//-----------------------------------------------------------------
int SpatialiteDbInfo::setDatabaseInvalid( QString sLayerName, QString errCause )
{
  mIsDbValid = false;
  if ( !errCause.isEmpty() )
  {
    mErrors.insert( sLayerName, errCause );
  }
  return errCause.size();
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::prepare
//-----------------------------------------------------------------
bool SpatialiteDbInfo::prepare()
{
  bool bChanged = false;
  if ( isDbValid() )
  {
    //----------------------------------------------------------
    // Set (what up to now was the LayerId-counter) to the found valid Layer-Count
    mLayersCount = mDbLayers.size();
    bChanged = prepareDataSourceUris();
    //---------------------------------------------------------------
    // QgsLayerMetadata for Database
    //---------------------------------------------------------------
    setLayerMetadata();
    //---------------------------------------------------------------
    if ( getSniffType() != SpatialiteDbInfo::SniffLoadLayers )
    {
      return bChanged;
    }
  }
  return bChanged;
};
//-----------------------------------------------------------------
// SpatialiteDbInfo::setLayerMetadata
//-----------------------------------------------------------------
bool SpatialiteDbInfo::setLayerMetadata()
{
  //---------------------------------------------------------------
  // QgsLayerMetadata forDatabase
  //---------------------------------------------------------------
  // complete Path (without without symbolic links)
  mDbMetadata.setIdentifier( getDatabaseUri() );
  mDbMetadata.setType( dbSpatialMetadataString() );
  mDbMetadata.setTitle( QString( "%1 with %2 Layers" ).arg( dbSpatialMetadataString() ).arg( dbLayersCount() ) );
  mDbMetadata.setAbstract( getSummary() );
  //---------------------------------------------------------------
  return isDbValid();
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::getDbLayerUris
//-----------------------------------------------------------------
QString SpatialiteDbInfo::getDbLayerUris( QString sLayerName )
{
  return mDbLayersDataSourceUris.value( sLayerName ); // QString() if not found
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::getDbLayerInfo
//-----------------------------------------------------------------
QString SpatialiteDbInfo::getDbLayerInfo( QString sLayerName )
{
  return mDbLayersDataSourceInfo.value( sLayerName ); // QString() if not found
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::prepareGroupLayers
//-----------------------------------------------------------------
int SpatialiteDbInfo::prepareGroupLayers()
{
  mListGroupNames.sort();
  QStringList sa_ListGroupNames = mListGroupNames;
  mListGroupNames.clear();
  mMapGroupNames.clear();
  //---------------------------------------------------------------
  for ( int i = 0; i < sa_ListGroupNames.size(); ++i )
  {
    if ( !mListGroupNames.contains( sa_ListGroupNames.at( i ) ) )
    {
      QStringList sa_result;
      sa_result = sa_ListGroupNames.filter( sa_ListGroupNames.at( i ) );
      if ( sa_result.size() > 1 )
      {
        QString sListGroupName = sa_ListGroupNames.at( i );
        mListGroupNames.append( sListGroupName );
        // Remove last '_' for mMapGroupNames
        sListGroupName.chop( 1 );
        mMapGroupNames.insert( sListGroupName, sa_result.size() );
      }
    }
  }
  //---------------------------------------------------------------
  return mListGroupNames.size();
};
//-----------------------------------------------------------------
// SpatialiteDbInfo::createDbLayerInfoUri
//-----------------------------------------------------------------
// Self contained function to create Uri + LayerInfo for all types
// -> Spatialite: 'PathToFile table="table_name" (geometry_name)'
// --> LayerInfo: 'LineString;3035;spatialite;SpatialView;1;0'
// -> RasterLite2: [TODO] 'PathToFile table="coverage_name"'
// --> LayerInfo: 'RasterLite2Raster;3035;rasterlite2;RasterLite2Raster;-1;0'
// -> RasterLite1 [Gdal]: 'RASTERLITE:/long/path/to/database/ItalyRail.atlas,table=srtm'
// --> LayerInfo: ''
// -> MBTiles [Gdal]: 'PathToFile'
// --> LayerInfo: ''
// -> FdoOgr [Ogr]:  'PathToFile|layername=table_name(geometry)'
// --> LayerInfo: ''
// -> GeoPackage [Ogr]: 'PathToFile|layername=table_name'
// --> LayerInfo: ''
//-----------------------------------------------------------------
QString SpatialiteDbInfo::createDbLayerInfoUri( QString &sLayerInfo, QString &sDataSourceUri, QString sLayerName,
    SpatialiteDbInfo::SpatialiteLayerType layerType, QString sGeometryType, int iSrid, int iSpatialIndex, int iIsHidden )
{
  QString sDataSourceUriBase = QString( "%1 table='%2'" );
  QString sTableName  = QString::null;
  QString sGeometryColumn = QString::null;
  QString sLayerType = SpatialiteDbInfo::SpatialiteLayerTypeName( layerType );
//----------------------------------------------------------
  // Extract TableName/GeometryColumn from sent 'table_name' or 'table_name(field_name)' from LayerName
  SpatialiteDbInfo::parseLayerName( sLayerName, sTableName, sGeometryColumn );
//----------------------------------------------------------
  switch ( layerType )
  {
    case SpatialiteDbInfo::SpatialTable:
    case SpatialiteDbInfo::SpatialView:
    case SpatialiteDbInfo::VirtualShape:
    case SpatialiteDbInfo::TopologyExport:
    case SpatialiteDbInfo::SpatialiteTopology:
    {
      sLayerInfo = QString( "%2%1%3%1%4%1%5%1%6%1%7" ).arg( SpatialiteDbInfo::ParseSeparatorGeneral ).arg( sGeometryType ).arg( iSrid ).arg( mSpatialiteProviderKey ).arg( sLayerType ).arg( iSpatialIndex ).arg( iIsHidden );
      sDataSourceUri = QString( sDataSourceUriBase.arg( getDatabaseUri() ).arg( sTableName ) );
      if ( !sGeometryColumn.isEmpty() )
      {
        sDataSourceUri += QString( " (%1)" ).arg( sGeometryColumn );
      }
    }
    break;
    case SpatialiteDbInfo::RasterLite2Raster:
    {
      // Note: the gdal notation will be retained, so that only one connection string exists. QgsDataSourceUri does not work with this gdal version [will fail].
      sLayerInfo = QString( "%2%1%3%1%4%1%5%1%6%1%7" ).arg( SpatialiteDbInfo::ParseSeparatorGeneral ).arg( sGeometryType ).arg( iSrid ).arg( mRasterLite2ProviderKey ).arg( sLayerType ).arg( iSpatialIndex ).arg( iIsHidden );
      sDataSourceUri = QString( "%4%1%2%1%3" ).arg( SpatialiteDbInfo::ParseSeparatorUris ).arg( getDatabaseFileName() ).arg( sTableName ).arg( QStringLiteral( "RASTERLITE2" ) );
    }
    break;
    case SpatialiteDbInfo::RasterLite1:
    {
      sLayerInfo = QString( "%2%1%3%1%4%1%5%1%6%1%7" ).arg( SpatialiteDbInfo::ParseSeparatorGeneral ).arg( sGeometryType ).arg( iSrid ).arg( mGdalProviderKey ).arg( sLayerType ).arg( iSpatialIndex ).arg( iIsHidden );
      sDataSourceUriBase = QString( "%1,table=%2" );
      sDataSourceUri = QString( sDataSourceUriBase.arg( QString( "%3%1%2" ).arg( SpatialiteDbInfo::ParseSeparatorUris ).arg( getDatabaseFileName() ).arg( QStringLiteral( "RASTERLITE" ) ) ).arg( sTableName ) );
    }
    break;
    case SpatialiteDbInfo::GdalFdoOgr:
    {
      sLayerInfo = QString( "%2%1%3%1%4%1%5%1%6%1%7" ).arg( SpatialiteDbInfo::ParseSeparatorGeneral ).arg( sGeometryType ).arg( iSrid ).arg( mOgrProviderKey ).arg( sLayerType ).arg( iSpatialIndex ).arg( iIsHidden );
      sDataSourceUri = QString( "%1|%2=%3" ).arg( getDatabaseFileName() ).arg( QStringLiteral( "layername" ) ).arg( sTableName );
    }
    break;
    case SpatialiteDbInfo::GeoPackageVector:
    {
      sLayerInfo = QString( "%2%1%3%1%4%1%5%1%6%1%7" ).arg( SpatialiteDbInfo::ParseSeparatorGeneral ).arg( sGeometryType ).arg( iSrid ).arg( mOgrProviderKey ).arg( sLayerType ).arg( iSpatialIndex ).arg( iIsHidden );
      sDataSourceUri = QString( "%1|%3=%2" ).arg( getDatabaseFileName() ).arg( sTableName ).arg( QStringLiteral( "layername" ) );
    }
    break;
    case SpatialiteDbInfo::GeoPackageRaster:
    {
      sLayerInfo = QString( "%2%1%3%1%4%1%5%1%6%1%7" ).arg( SpatialiteDbInfo::ParseSeparatorGeneral ).arg( sGeometryType ).arg( iSrid ).arg( mGdalProviderKey ).arg( sLayerType ).arg( iSpatialIndex ).arg( iIsHidden );
      sDataSourceUri = QString( "%4%1%2%1%3" ).arg( SpatialiteDbInfo::ParseSeparatorUris ).arg( getDatabaseFileName() ).arg( sTableName ).arg( QStringLiteral( "GPKG" ) );
    }
    break;
    case SpatialiteDbInfo::MBTilesTable:
    case SpatialiteDbInfo::MBTilesView:
    {
      sLayerInfo = QString( "%2%1%3%1%4%1%5%1%6%1%7" ).arg( SpatialiteDbInfo::ParseSeparatorGeneral ).arg( sGeometryType ).arg( iSrid ).arg( mGdalProviderKey ).arg( sLayerType ).arg( iSpatialIndex ).arg( iIsHidden );
      sDataSourceUri = getDatabaseFileName();
    }
    break;
    case SpatialiteDbInfo::StyleVector:
    case SpatialiteDbInfo::StyleRaster:
    case SpatialiteDbInfo::NonSpatialTables:
    case SpatialiteDbInfo::AllLayers:
    case SpatialiteDbInfo::AllSpatialLayers:
    case SpatialiteDbInfo::RasterLite2Vector:
    default:
      sDataSourceUri = QString();
      sLayerInfo = QString();
      break;
  }
//----------------------------------------------------------
  return sLayerInfo;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::parseLayerInfo
//-----------------------------------------------------------------
// Seperator: ';' [SpatialiteDbInfo::ParseSeparatorGeneral]
// Expected fields: 6
// - [0] GeometryType or LayerType
// - [1]  Srid
// - [2]  Provider
// - [3]  LayerType
// - [4]  SpatialIndex
// - [5]  IsHidden
//-----------------------------------------------------------------
bool SpatialiteDbInfo::parseLayerInfo( QString sLayerInfo, QString &sGeometryType, int &iSrid, QString &sProvider, QString &sLayerType, int &iSpatialIndex, int &iIsHidden )
{
  bool bRc = false;
  QStringList sa_list_info = sLayerInfo.split( SpatialiteDbInfo::ParseSeparatorGeneral );
  if ( sa_list_info.size() == 6 )
  {
    // For Layers that contain no Geometries, this will be the Layer-Type
    sGeometryType = sa_list_info.at( 0 );
    iSrid = sa_list_info.at( 1 ).toInt();
    sProvider = sa_list_info.at( 2 );
    // Needed by QgsSpatialiteLayerItem, where no SpatialiteDbLayer exists
    sLayerType = sa_list_info.at( 3 );
    iSpatialIndex = sa_list_info.at( 4 ).toInt();
    iIsHidden = sa_list_info.at( 5 ).toInt();
    bRc = true;
  }
  return bRc;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::parseLayerCoverage
//-----------------------------------------------------------------
// Seperator: '€' [SpatialiteDbInfo::ParseSeparatorCoverage]
// Expected fields: 7
// - [0] LayerType
// - [1]  LayerName
// - [2]  Title
// - [3]  Abstract
// - [4]  Copyright
// - [5]  License
// - [6]  Srid
//-----------------------------------------------------------------
bool SpatialiteDbInfo::parseLayerCoverage( QString sLayerInfo, QString &sLayerType,  QString &sLayerName, QString &sTitle, QString &sAbstract, QString &sCopyright, QString &sLicense, int &iSrid )
{
  bool bRc = false;
  QStringList sa_list_info = sLayerInfo.split( SpatialiteDbInfo::ParseSeparatorCoverage );
  if ( sa_list_info.size() == 7 )
  {
    // For Layers  this will be the Layer-Type
    sLayerType = sa_list_info.at( 0 );
    sLayerName = sa_list_info.at( 1 );
    sTitle = sa_list_info.at( 2 );
    sAbstract = sa_list_info.at( 3 );
    sCopyright = sa_list_info.at( 4 );
    sLicense = sa_list_info.at( 5 );
    iSrid = sa_list_info.at( 6 ).toInt();
    bRc = true;
  }
  return bRc;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::prepareDataSourceUris
//-----------------------------------------------------------------
// Goal is to produce the same Uris and LayerInfo strings
// - independent of the Layer Type
// -> outside sources [QgsSpatialiteLayerItem] will call, with the LayerName:
// --> getDbLayerInfo or getDbLayerUris
// Any changes here must be done with great care.
//-----------------------------------------------------------------
bool SpatialiteDbInfo::prepareDataSourceUris()
{
  bool bChanged = false;
  //---------------------------------------------------------------
  mDbLayersDataSourceUris.clear();
  // getDatabaseUri() will insure that the Path-Name is properly formatted
  QString sDataSourceUri;
  QString sLayerName;
  QString sLayerInfo;
  QString sGeometryType;
  QString sLayerType;
  int iSrid;
  QString sProvider;
  int iSpatialIndex = GAIA_VECTOR_UNKNOWN;
  int iIsHidden = 0;
  QString sTitle;
  QString sAbstract;
  QString sCopyright;
  QString sLicense;
  for ( QMap<QString, QString>::iterator itLayers = mVectorLayers.begin(); itLayers != mVectorLayers.end(); ++itLayers )
  {
    if ( parseLayerInfo( itLayers.value(), sGeometryType, iSrid, sProvider, sLayerType, iSpatialIndex, iIsHidden ) )
    {
      createDbLayerInfoUri( sLayerInfo, sDataSourceUri, itLayers.key(), SpatialiteDbInfo::SpatialTable, sGeometryType, iSrid, iSpatialIndex, iIsHidden );
      mDbLayersDataSourceUris.insert( itLayers.key(), sDataSourceUri );
      mDbLayersDataSourceInfo.insert( itLayers.key(), itLayers.value() );
    }
  }
  for ( QMap<QString, QString>::iterator itLayers = mRasterCoveragesLayers.begin(); itLayers != mRasterCoveragesLayers.end(); ++itLayers )
  {
    if ( parseLayerCoverage( itLayers.value(), sLayerType, sLayerName, sTitle, sAbstract, sCopyright, sLicense, iSrid ) )
    {
      iSpatialIndex = GAIA_VECTOR_UNKNOWN;
      iIsHidden = 0;
      createDbLayerInfoUri( sLayerInfo, sDataSourceUri, itLayers.key(), SpatialiteDbInfo::RasterLite2Raster, sLayerType, iSrid, iSpatialIndex, iIsHidden );
      mDbLayersDataSourceUris.insert( itLayers.key(), sDataSourceUri );
      mDbLayersDataSourceInfo.insert( itLayers.key(), sLayerInfo );
    }
  }
  for ( QMap<QString, QString>::iterator itLayers = mTopologyExportLayers.begin(); itLayers != mTopologyExportLayers.end(); ++itLayers )
  {
    if ( parseLayerInfo( itLayers.value(), sGeometryType, iSrid, sProvider, sLayerType, iSpatialIndex, iIsHidden ) )
    {
      createDbLayerInfoUri( sLayerInfo, sDataSourceUri, itLayers.key(), SpatialiteDbInfo::TopologyExport, sGeometryType, iSrid, iSpatialIndex, iIsHidden );
      mDbLayersDataSourceUris.insert( itLayers.key(), sDataSourceUri );
      mDbLayersDataSourceInfo.insert( itLayers.key(), itLayers.value() );
    }
  }
  for ( QMap<QString, QString>::iterator itLayers = mRasterLite1Layers.begin(); itLayers != mRasterLite1Layers.end(); ++itLayers )
  {
    if ( parseLayerInfo( itLayers.value(), sGeometryType, iSrid, sProvider, sLayerType, iSpatialIndex, iIsHidden ) )
    {
      createDbLayerInfoUri( sLayerInfo, sDataSourceUri, itLayers.key(), SpatialiteDbInfo::RasterLite1, sGeometryType, iSrid, iSpatialIndex, iIsHidden );
      mDbLayersDataSourceUris.insert( itLayers.key(), sDataSourceUri );
      mDbLayersDataSourceInfo.insert( itLayers.key(), itLayers.value() );
    }
  }
  for ( QMap<QString, QString>::iterator itLayers = mFdoOgrLayers.begin(); itLayers != mFdoOgrLayers.end(); ++itLayers )
  {
    if ( parseLayerInfo( itLayers.value(), sGeometryType, iSrid, sProvider, sLayerType, iSpatialIndex, iIsHidden ) )
    {
      createDbLayerInfoUri( sLayerInfo, sDataSourceUri, itLayers.key(), SpatialiteDbInfo::GdalFdoOgr, sGeometryType, iSrid, iSpatialIndex, iIsHidden );
      mDbLayersDataSourceUris.insert( itLayers.key(), sDataSourceUri );
      mDbLayersDataSourceInfo.insert( itLayers.key(), itLayers.value() );
    }
  }
  for ( QMap<QString, QString>::iterator itLayers = mGeoPackageLayers.begin(); itLayers != mGeoPackageLayers.end(); ++itLayers )
  {
    SpatialiteDbInfo::SpatialiteLayerType layerType = SpatialiteDbInfo::GeoPackageVector;
    if ( parseLayerInfo( itLayers.value(), sGeometryType, iSrid, sProvider, sLayerType, iSpatialIndex, iIsHidden ) )
    {
      if ( sGeometryType == "GeoPackageRaster" )
      {
        layerType = SpatialiteDbInfo::GeoPackageRaster;
      }
      createDbLayerInfoUri( sLayerInfo, sDataSourceUri, itLayers.key(), layerType, sGeometryType, iSrid, iSpatialIndex, iIsHidden );
      mDbLayersDataSourceUris.insert( itLayers.key(), sDataSourceUri );
      mDbLayersDataSourceInfo.insert( itLayers.key(), itLayers.value() );
    }
  }
  for ( QMap<QString, QString>::iterator itLayers = mMBTilesLayers.begin(); itLayers != mMBTilesLayers.end(); ++itLayers )
  {
    if ( parseLayerInfo( itLayers.value(), sGeometryType, iSrid, sProvider, sLayerType, iSpatialIndex, iIsHidden ) )
    {
      createDbLayerInfoUri( sLayerInfo, sDataSourceUri, itLayers.key(), SpatialiteDbInfo::MBTilesTable, sGeometryType, iSrid, iSpatialIndex, iIsHidden );
      mDbLayersDataSourceUris.insert( itLayers.key(), sDataSourceUri );
      mDbLayersDataSourceInfo.insert( itLayers.key(), itLayers.value() );
    }
  }
  if ( mDbLayersDataSourceUris.size() > 0 )
  {
    bChanged = true;
    mIsDbValid = true;
  }
  //---------------------------------------------------------------
  return bChanged;
};
//-----------------------------------------------------------------
// SpatialiteDbInfo::getDbLayersType
//-----------------------------------------------------------------
QMap<QString, QString> SpatialiteDbInfo::getDbLayersType( SpatialiteLayerType typeLayer )
{
  QMap<QString, QString> mapLayers;
  //---------------------------------------------------------------
  switch ( typeLayer )
  {
    case SpatialiteDbInfo::SpatialTable:
    case SpatialiteDbInfo::SpatialView:
    case SpatialiteDbInfo::VirtualShape:
    {
      // Key : LayerType SpatialTable, SpatialView and VirtualShape
      // Value: LayerName formatted as 'table_name(geometry_name)' or 'table_name'
      QList<QString> sa_list = mVectorLayersTypes.keys( SpatialiteDbInfo::SpatialiteLayerTypeName( typeLayer ) );
      for ( int i = 0; i < sa_list.size(); ++i )
      {
        // Return a list that only contains the given type
        mapLayers.insert( sa_list.at( i ), mVectorLayers.value( sa_list.at( i ) ) );
      }
    }
    break;
    case SpatialiteDbInfo::RasterLite1:
      mapLayers = getDbRasterLite1Layers();
      break;
    case SpatialiteDbInfo::RasterLite2Vector:
      mapLayers = getDbVectorCoveragesLayers();
      break;
    case SpatialiteDbInfo::RasterLite2Raster:
      mapLayers = getDbRasterCoveragesLayers();
      break;
    case SpatialiteDbInfo::TopologyExport:
      mapLayers = getDbTopologyExportLayers();
      break;
    case SpatialiteDbInfo::GeoPackageVector:
    case SpatialiteDbInfo::GeoPackageRaster:
      mapLayers = getDbGeoPackageLayers();
      break;
    case SpatialiteDbInfo::MBTilesTable:
    case SpatialiteDbInfo::MBTilesView:
      mapLayers = getDbMBTilesLayers();
      break;
    case SpatialiteDbInfo::NonSpatialTables:
      mapLayers = getDbNonSpatialTables();
      break;
    case SpatialiteDbInfo::AllSpatialLayers:
    case SpatialiteDbInfo::AllLayers:
    {
      for ( QMap<QString, QString>::iterator itLayers = mVectorLayers.begin(); itLayers != mVectorLayers.end(); ++itLayers )
      {
        mapLayers.insert( itLayers.key(), itLayers.value() );
      }
      if ( dbVectorCoveragesLayersCount() > 0 )
      {
        for ( QMap<QString, QString>::iterator itLayers = mVectorCoveragesLayers.begin(); itLayers != mVectorCoveragesLayers.end(); ++itLayers )
        {
          mapLayers.insert( itLayers.key(), itLayers.value() );
        }
      }
      if ( dbRasterCoveragesLayersCount() > 0 )
      {
        for ( QMap<QString, QString>::iterator itLayers = mRasterCoveragesLayers.begin(); itLayers != mRasterCoveragesLayers.end(); ++itLayers )
        {
          mapLayers.insert( itLayers.key(), itLayers.value() );
        }
      }
      if ( dbRasterLite1LayersCount() > 0 )
      {
        for ( QMap<QString, QString>::iterator itLayers = mRasterLite1Layers.begin(); itLayers != mRasterLite1Layers.end(); ++itLayers )
        {
          mapLayers.insert( itLayers.key(), itLayers.value() );
        }
      }
      if ( dbTopologyExportLayersCount() > 0 )
      {
        for ( QMap<QString, QString>::iterator itLayers = mTopologyExportLayers.begin(); itLayers != mTopologyExportLayers.end(); ++itLayers )
        {
          mapLayers.insert( itLayers.key(), itLayers.value() );
        }
      }
      if ( dbGeoPackageLayersCount() > 0 )
      {
        for ( QMap<QString, QString>::iterator itLayers = mGeoPackageLayers.begin(); itLayers != mGeoPackageLayers.end(); ++itLayers )
        {
          mapLayers.insert( itLayers.key(), itLayers.value() );
        }
      }
      if ( dbFdoOgrLayersCount() > 0 )
      {
        for ( QMap<QString, QString>::iterator itLayers = mFdoOgrLayers.begin(); itLayers != mFdoOgrLayers.end(); ++itLayers )
        {
          mapLayers.insert( itLayers.key(), itLayers.value() );
        }
      }
      if ( dbMBTilesLayersCount() > 0 )
      {
        for ( QMap<QString, QString>::iterator itLayers = mMBTilesLayers.begin(); itLayers != mMBTilesLayers.end(); ++itLayers )
        {
          mapLayers.insert( itLayers.key(), itLayers.value() );
        }
      }
      if ( ( typeLayer == SpatialiteDbInfo::AllLayers ) && ( dbNonSpatialTablesCount() > 0 ) )
      {
        for ( QMap<QString, QString>::iterator itLayers = mNonSpatialTables.begin(); itLayers != mNonSpatialTables.end(); ++itLayers )
        {
          mapLayers.insert( itLayers.key(), itLayers.value() );
        }
      }
    }
    break;
    default:
      break;
  }
  //---------------------------------------------------------------
  return mapLayers;
};
//-----------------------------------------------------------------
// SpatialiteDbInfo::getSpatialiteDbLayer [public]
// - this can be called from outside
// - but can also be called from GetDbLayersInfo
// -> which can cause a Loop
// GetDbLayersInfo will determin if the give Layername exists
// - bFoundLayerName will return true if found and Layername not empty
// - if GetDbLayersInfo returns false: the Layer contains errors and cannot be created
// -> bail out avoiding a possible Loop, returning nullptr
//-----------------------------------------------------------------
SpatialiteDbLayer *SpatialiteDbInfo::getSpatialiteDbLayer( QString sLayerName, bool loadLayer )
{
  bool bFoundLayerName = false;
  //---------------------------------------------------------------
  if ( !sLayerName.isEmpty() )
  {
    // Note: for some reason this is being called with the uri ; extract the LayerName in such cases
    if ( ( sLayerName.contains( "dbname=" ) ) && ( sLayerName.contains( "table=" ) ) )
    {
      QgsDataSourceUri anUri = QgsDataSourceUri( sLayerName );
      sLayerName = anUri.table();
      if ( !anUri.geometryColumn().toLower().isEmpty() )
      {
        sLayerName = QString( "%1(%2)" ).arg( sLayerName ).arg( anUri.geometryColumn().toLower() );
      }
    }
  }
  //---------------------------------------------------------------
  // Retrieve the Layer, if allready exists
  SpatialiteDbLayer *dbLayer = qobject_cast<SpatialiteDbLayer *>( mDbLayers.value( sLayerName ) );
  if ( dbLayer )
  {
    return dbLayer;
  }
  else
  {
    if ( loadLayer )
    {
      // Create the Layer, when not found
      if ( GetDbLayersInfo( sLayerName, bFoundLayerName ) )
      {
        // Retrieve the Layer, which should now exist
        dbLayer = getSpatialiteDbLayer( sLayerName, false );
        if ( dbLayer )
        {
          return dbLayer;
        }
      }
      else
      {
        dbLayer = nullptr;
        //---------------------------------------------------------------
        if ( ( !sLayerName.isEmpty() ) && ( bFoundLayerName ) )
        {
          // The LayerName was found in one of the Map-Lists
          // -> the Layer contains errors and cannot be created
          return dbLayer; // return, avoiding loop
        }
      }
    }
  }
  //---------------------------------------------------------------
  if ( ! mDbLayers.contains( sLayerName ) )
  {
    if ( mDbLayersDataSourceUris.contains( sLayerName ) )
    {
      // a DataSourceUris is being sent as LayerName, replace with Layer-Name
      sLayerName = mDbLayersDataSourceUris.value( sLayerName );
    }
  }
  //---------------------------------------------------------------
  if ( mDbLayers.contains( sLayerName ) )
  {
    dbLayer = qobject_cast<SpatialiteDbLayer *>( mDbLayers.value( sLayerName ) );
    if ( dbLayer )
    {
      return dbLayer;
    }
  }
  //---------------------------------------------------------------
  if ( loadLayer )
  {
    if ( GetDbLayersInfo( sLayerName, bFoundLayerName ) )
    {
      dbLayer = getSpatialiteDbLayer( sLayerName, loadLayer );
      if ( dbLayer )
      {
        return dbLayer;
      }
    }
    dbLayer = nullptr;
  }
  //---------------------------------------------------------------
  return dbLayer;
};
//-----------------------------------------------------------------
// SpatialiteDbInfo::addDbMapLayers
//-----------------------------------------------------------------
int SpatialiteDbInfo::addDbMapLayers( QStringList saSelectedLayers, QStringList saSelectedLayersSql )
{
  mSelectedLayersUris.clear();
  QList<QgsMapLayer *> mapLayersVector;
  QList<QgsMapLayer *> mapLayersPolygons;
  QList<QgsMapLayer *> mapLayersLinestrings;
  QList<QgsMapLayer *> mapLayersPoints;
  QList<QgsMapLayer *> mapLayersRaster;
  bool bLoadLayer = true;
  QString sGeometryType;
  int iSrid;
  QString sProvider;
  QString sLayerType;
  int i_count_added = 0;
  int iSpatialIndex = GAIA_VECTOR_UNKNOWN;
  int iIsHidden = 0;
  //---------------------------------------------------------------
  for ( int i = 0; i < saSelectedLayers.count(); i++ )
  {
    QString sLayerNameSql;
    QString sLayerName = saSelectedLayers.at( i );
    if ( sLayerName == getDatabaseFileName() )
    {
      QList<QString> sa_list = getDataSourceUris().keys();
      if ( sa_list.size() )
      {
        // this is a Drag and Drop action, load only first layer
        sLayerName = sa_list.at( 0 );
      }
    }
    SpatialiteDbLayer *dbLayer = getSpatialiteDbLayer( sLayerName, bLoadLayer );
    if ( dbLayer )
    {
      // LayerInfo formatted as 'geometry_type:srid:provider:layertype'
      if ( ( dbLayer->isLayerValid() ) && ( parseLayerInfo( dbLayer->getLayerInfo(), sGeometryType, iSrid, sProvider, sLayerType, iSpatialIndex, iIsHidden ) ) )
      {
        // LayerName formatted as 'table_name(geometry_name)' or 'table_name' or 'table_name'
        if ( i < saSelectedLayersSql.count() )
        {
          sLayerNameSql = saSelectedLayersSql.at( i );
        }
        QgsVectorLayer *vectorLayer = nullptr;
        QgsRasterLayer *rasterLayer = nullptr;
        QString sLayerDataSourceUri = dbLayer->getLayerDataSourceUri();
        if ( sProvider == mSpatialiteProviderKey )
        {
          if ( !sLayerNameSql.isEmpty() )
          {
            sLayerDataSourceUri = QString( "%1 sql=%2" ).arg( sLayerDataSourceUri ).arg( sLayerNameSql );
          }
          vectorLayer = new QgsVectorLayer( sLayerDataSourceUri, sLayerName, mSpatialiteProviderKey );
          if ( vectorLayer->isValid() )
          {
            if ( ( dbLayer ) && ( dbLayer->isLayerValid() ) && ( dbLayer->hasLayerStyle() ) )
            {
              QString errorMessage;
              QDomElement namedLayerElement = dbLayer->getLayerStyleNamedLayerElement( dbLayer->getLayerStyleSelectedId(), errorMessage );
              if ( !namedLayerElement.isNull() )
              {
                QDomElement nameElement = namedLayerElement.firstChildElement( QStringLiteral( "Name" ) );
                if ( !nameElement.isNull() )
                {
                  if ( !vectorLayer->readSld( namedLayerElement, errorMessage ) )
                  {
                    QgsDebugMsgLevel( QString( "[Style cannot be rendered]  LayerName[%1]: StyleName[%2] error[%3]" ).arg( sLayerName ).arg( dbLayer->getLayerStyleSelected() ).arg( errorMessage ), 3 );
                  }
                  else
                  {
                    QgsDebugMsgLevel( QString( "[Style will be rendered]  LayerName[%1]: StyleName[%2] " ).arg( sLayerName ).arg( dbLayer->getLayerStyleSelected() ), 3 );
                  }
                }
                else
                {
                  QgsDebugMsgLevel( QString( "[Style missing Name Element]  LayerName[%1]: StyleName[%2] " ).arg( sLayerName ).arg( dbLayer->getLayerStyleSelected() ), 3 );
                }
              }
              else
              {
                QgsDebugMsgLevel( QString( "[Style retrieving NamedLayerElement failed]  LayerName[%1]: StyleName[%2] error[%3]" ).arg( sLayerName ).arg( dbLayer->getLayerStyleSelected() ).arg( errorMessage ), 3 );
              }
            }
          }
        }
        else if ( sProvider == mOgrProviderKey )
        {
          if ( !sLayerNameSql.isEmpty() )
          {
            sLayerDataSourceUri = QString( "%1 sql=%2" ).arg( sLayerDataSourceUri ).arg( sLayerNameSql );
          }
          vectorLayer = new QgsVectorLayer( sLayerDataSourceUri, sLayerName, mOgrProviderKey );
        }
        else  if ( sProvider == mGdalProviderKey )
        {
          bool bCheckDriver = true;
          if ( ( sLayerDataSourceUri.startsWith( "RASTERLITE:" ) ) && ( !hasDbGdalRasterLite1Driver() ) )
          {
            // Do not load RasterLite1-Layer, if the Gdal-Driver was not found
            bCheckDriver = true;
          }
          if ( bCheckDriver )
          {
            rasterLayer = new QgsRasterLayer( sLayerDataSourceUri, sLayerName, sProvider );
          }
        }
        else  if ( sProvider == mRasterLite2ProviderKey )
        {
          if ( dbHasRasterlite2() )
          {
            sProvider = mRasterLite2ProviderKey;
          }
          else
          {
            sProvider = mGdalProviderKey;
          }
          // Note: at present (2017-07-31) QGdalProvider cannot display RasterLite2-Rasters created with the development version
          rasterLayer = new QgsRasterLayer( sLayerDataSourceUri, sLayerName, sProvider );
        }
        if ( vectorLayer )
        {
          if ( !vectorLayer->isValid() )
          {
            delete vectorLayer;
            continue;
          }
          vectorLayer->setShortName( dbLayer->getLayerName() );
          vectorLayer->setTitle( dbLayer->getTitle() );
          vectorLayer->setAbstract( dbLayer->getAbstract() );
          vectorLayer->setKeywordList( dbLayer->getCopyright() );
          // Set collected Metadata for the Layer
          // vectorLayer->setMetadata( dbLayer->getLayerMetadata() );
          switch ( dbLayer->getGeometryType() )
          {
            case QgsWkbTypes::Point:
            case QgsWkbTypes::Point25D:
            case QgsWkbTypes::MultiPoint:
            case QgsWkbTypes::MultiPoint25D:
              mapLayersPoints << vectorLayer;
              break;
            case QgsWkbTypes::LineString:
            case QgsWkbTypes::LineString25D:
            case QgsWkbTypes::MultiLineString:
            case QgsWkbTypes::MultiLineString25D:
              mapLayersLinestrings << vectorLayer;
              break;
            case QgsWkbTypes::Polygon:
            case QgsWkbTypes::Polygon25D:
            case QgsWkbTypes::MultiPolygon:
            case QgsWkbTypes::MultiPolygon25D:
            default:
              mapLayersPolygons  << vectorLayer;
              break;
          }
          mapLayersVector << vectorLayer;
        }
        if ( rasterLayer )
        {
          if ( !rasterLayer->isValid() )
          {
            delete rasterLayer;
            continue;
          }
          rasterLayer->setShortName( dbLayer->getLayerName() );
          rasterLayer->setTitle( dbLayer->getTitle() );
          rasterLayer->setAbstract( dbLayer->getAbstract() );
          rasterLayer->setKeywordList( dbLayer->getCopyright() );
          // Set collected Metadata for the Layer
          // rasterLayer->setMetadata( dbLayer->getLayerMetadata() );
          mapLayersRaster << rasterLayer;
        }
      }
    }
  }
  //---------------------------------------------------------------
  // First load the Rasters, then the Vectors
  // - so that the Vectors will be on top of the Rasters
  // --> the last appended, is rendered first
  //---------------------------------------------------------------
  if ( mapLayersRaster.count() > 0 )
  {
    mapLayersVector.append( mapLayersRaster );
    i_count_added += mapLayersRaster.count();
    mapLayersRaster.clear();
  }
  //---------------------------------------------------------------
  // and add any Polygons
  // - so that any Linestrings / Points are over the Polygons
  // --> the last appended, is rendered first
  //---------------------------------------------------------------
  if ( mapLayersPolygons.count() > 0 )
  {
    mapLayersVector.append( mapLayersPolygons );
    i_count_added += mapLayersPolygons.count();
    mapLayersPolygons.clear();
  }
  //---------------------------------------------------------------
  // Send the MapLayers to Qgis
  // -> the order for Vectors-Types does not always work
  // --> so will send this piecemeal, any Rasters will be rendered first
  //---------------------------------------------------------------
  if ( mapLayersVector.count() > 0 )
  {
    // so that layer is added to legend [first boolean parm]
    QgsProject::instance()->addMapLayers( mapLayersVector, true, false );
    i_count_added += mapLayersVector.count();
    mapLayersVector.clear();
  }
  //---------------------------------------------------------------
  // and then any Linestrings
  // - so that they are under the Points
  // --> the last appended, is rendered first
  //---------------------------------------------------------------
  if ( mapLayersLinestrings.count() > 0 )
  {
    mapLayersVector.append( mapLayersLinestrings );
    i_count_added += mapLayersLinestrings.count();
    mapLayersLinestrings.clear();
    // so that layer is added to legend [first boolean parm]
    QgsProject::instance()->addMapLayers( mapLayersVector, true, false );
    mapLayersVector.clear();
  }
  //---------------------------------------------------------------
  //  and as last the Points
  // - everything should now be seen
  // --> everything selected should now be viewable
  //---------------------------------------------------------------
  if ( mapLayersPoints.count() > 0 )
  {
    mapLayersVector.append( mapLayersPoints );
    i_count_added += mapLayersPoints.count();
    mapLayersPoints.clear();
    // so that layer is added to legend [first boolean parm]
    QgsProject::instance()->addMapLayers( mapLayersVector, true, false );
    i_count_added += mapLayersVector.count();
    mapLayersVector.clear();
  }
  //---------------------------------------------------------------
  return i_count_added;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::getSpatialiteDbLayer
//-----------------------------------------------------------------
SpatialiteDbLayer *SpatialiteDbInfo::getSpatialiteDbLayer( int layerId )
{
  SpatialiteDbLayer *dbLayer = nullptr;
  //---------------------------------------------------------------
  for ( QMap<QString, SpatialiteDbLayer *>::iterator it = mDbLayers.begin(); it != mDbLayers.end(); ++it )
  {
    SpatialiteDbLayer *searchLayer = qobject_cast<SpatialiteDbLayer *>( it.value() );
    if ( searchLayer )
    {
      if ( searchLayer->mLayerId == layerId )
      {
        return searchLayer;
      }
    }
  }
  //---------------------------------------------------------------
  return dbLayer;
};
//-----------------------------------------------------------------
// SpatialiteDbInfo::getSpatialiteVersion
//-----------------------------------------------------------------
bool SpatialiteDbInfo::getSpatialiteVersion()
{
  // -- ---------------------------------- --
  // -- This must be a Sqlite3 Database
  // -- ---------------------------------- --
  if ( isDbSqlite3() )
  {
    if ( !getQSqliteHandle()->isDbSpatialiteActive() )
    {
      if ( !getQSqliteHandle()->loadSpatialite() )
      {
        return getQSqliteHandle()-> isDbSpatialiteActive(); // false
      }
      // the Spatialite connection should now be usable
    }
    sqlite3_stmt *stmt = nullptr;
    QString sql = QStringLiteral( "SELECT CheckSpatialMetadata(), spatialite_version()" );
    int i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
        {
          // -- ---------------------------------- --
          // Check and set known values
          // -> considered valid
          // - if not known, will check for MBTiles
          // -> if found considered valid since MBTiles is supported by QgsGdalProvider
          // -- ---------------------------------- --
          setSpatialMetadata( sqlite3_column_int( stmt, 0 ) );
        }
        if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
        {
          mSpatialiteVersionInfo = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
        }
      }
      sqlite3_finalize( stmt );
      if ( !mIsDbValid )
      {
        setDatabaseInvalid( getQSqliteHandle()->dbPath(), QString( "getSpatialiteLayerInfo: Invalid SpatialMetadata[%1]." ).arg( dbSpatialMetadataString() ) );
        return isDbValid();
      }
    }
    QString sVersionInfo = mSpatialiteVersionInfo;
    sVersionInfo.remove( QRegExp( QString::fromUtf8( "[-`~!@#$%^&*()_—+=|:;<>«»,?/{a-zA-Z}\'\"\\[\\]\\\\]" ) ) );
    QStringList spatialiteParts = sVersionInfo.split( ' ', QString::SkipEmptyParts );
    // Get major, minor and revision version
    QStringList spatialiteVersionParts = spatialiteParts[0].split( '.', QString::SkipEmptyParts );
    if ( spatialiteVersionParts.size() == 3 )
    {
      mSpatialiteVersionMajor = spatialiteVersionParts[0].toInt();
      mSpatialiteVersionMinor = spatialiteVersionParts[1].toInt();
      mSpatialiteVersionRevision = spatialiteVersionParts[2].toInt();
    }
    if ( mSpatialiteVersionMajor < 0 )
    {
      setDatabaseInvalid( getQSqliteHandle()->dbPath(), QString( "getSpatialiteLayerInfo: Invalid spatialite Major version[%1]." ).arg( mSpatialiteVersionMajor ) );
    }
    else
    {
      if ( mSpatialiteVersionMajor >= 4 )
      {
        if ( ( ( mSpatialiteVersionMajor == 4 ) && ( mSpatialiteVersionMinor >= 5 ) ) || ( mSpatialiteVersionMajor > 4 ) )
        {
          mIsVersion45 = true;
        }
      }
    }
    if ( mIsVersion45 )
    {
      sql = QStringLiteral( "SELECT HasTopology(), HasGCP()" ) ;
      i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
      if ( i_rc == SQLITE_OK )
      {
        while ( sqlite3_step( stmt ) == SQLITE_ROW )
        {
          if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
          {
            mHasTopology = sqlite3_column_int( stmt, 0 );
          }
          if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
          {
            mHasGcp = sqlite3_column_int( stmt, 1 );
          }
        }
        sqlite3_finalize( stmt );
      }
    }
  }
  return isDbValid();
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::getSpatialiteLayerInfo
//-----------------------------------------------------------------
bool SpatialiteDbInfo::getSpatialiteLayerInfo( QString sLayerName, bool bLoadLayers, SpatialSniff sniffType )
{
  if ( ( dbSqliteHandle() ) && ( isDbSqlite3() )  && ( isDbValid() ) )
  {
    mLoadLayers = bLoadLayers;
    if ( sniffType == SpatialiteDbInfo::SniffUnknown )
    {
      // Parameter was not give, so we will try to guess
      sniffType = SpatialiteDbInfo::SniffDatabaseType;
      if ( bLoadLayers )
      {
        sniffType = SpatialiteDbInfo::SniffLoadLayers;
      }
      else if ( !sLayerName.isEmpty() )
      {
        sniffType = SpatialiteDbInfo::SniffMinimal;
      }
    }
    setSniffType( sniffType );
    if ( !sLayerName.isEmpty() )
    {
      // If a specific Layer is being requested, only that Layer will be loaded into mDbLayers
      mLoadLayers = true;
    }
    if ( dbLayersCount() == 0 )
    {
      retrieveSniffMinimal();
      if ( ( getSniffType() == SpatialiteDbInfo::SniffMinimal ) || ( getSniffType() == SpatialiteDbInfo::SniffDatabaseType ) )
      {
        return isDbValid();
      }
    }
    // -- ---------------------------------- --
    // SpatialiteDbInfo::SniffLoadLayers
    // -- ---------------------------------- --
    // Clears mDbLayers [Collection of Loaded-layers]
    // Activating Foreign Key constraints for the Database
    //  Loads Layers being request based on value of sLayerName
    // -- ---------------------------------- --
    getSniffLoadLayers( sLayerName );
    // -- ---------------------------------- --
    QgsDebugMsgLevel( QString( "-I-> getSpatialiteLayerInfo[%23,%1] -SniffLoadLayers[%24]- IsValid[%17] IsSpatialite[%18] IsGdalOgr[%19]  dbLayersCount[%20]  VectorLayers[%2,%3] SpatialTables[%4] SpatialViews[%5] VirtualShapes[%6] VectorCoverages[%7] VectorCoveragesStyles[%8] RasterCoverages[%9] RasterCoveragesStyles[%10] RasterLite1[%11] TopologyExport[%12] FdoOgr[%13] GeoPackage[%14] MBTiles[%15] NonSpatial[%16] isDbSpatialiteActive[%21] isDbRasterLite2Active[%22] FileName[%25]" ).arg( dbSpatialMetadataString() ).arg( dbVectorLayersCount() ).arg( mHasLegacyGeometryLayers ).arg( dbSpatialTablesLayersCount() ).arg( dbSpatialViewsLayersCount() ).arg( dbVirtualShapesLayersCount() ).arg( dbVectorCoveragesLayersCount() ).arg( dbVectorStylesViewsCount() ).arg( dbRasterCoveragesLayersCount() ).arg( dbRasterStylesViewCount() ).arg( dbRasterLite1LayersCount() ).arg( dbTopologyExportLayersCount() ).arg( dbFdoOgrLayersCount() ).arg( dbGeoPackageLayersCount() ).arg( dbMBTilesLayersCount() ).arg( dbNonSpatialTablesCount() ).arg( isDbValid() ).arg( isDbSpatialite() ).arg( isDbGdalOgr() ).arg( dbLayersCount() ).arg( isDbSpatialiteActive() ).arg( isDbRasterLite2Active() ).arg( getSniffTypeString() ).arg( sLayerName ).arg( getFileName() ), 7 );
  }
  if ( getQSqliteHandle() )
  {
    getQSqliteHandle()->setStatus();
  }
  return isDbValid();
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::retrieveSniffMinimal
//-----------------------------------------------------------------
bool SpatialiteDbInfo::retrieveSniffMinimal()
{
  if ( ! getQSqliteHandle() )
  {
    return false;
  }
  //---------------------------------------------------------------
  if ( isDbValid() )
  {
    // SniffDatabaseType: determinee Sqlite3-Container-Type [SpatialMetadata]
    getSniffDatabaseType();
    if ( getSniffType() == SpatialiteDbInfo::SniffDatabaseType )
    {
      if ( getQSqliteHandle() )
      {
        getQSqliteHandle()->setStatus();
      }
      // -- ---------------------------------- --
      // For cases where only the Sqlite3-Container-Type and the Spatialite Version numbers are needed
      // -- ---------------------------------- --
      return isDbValid();
    }
    // SniffMinimal: Load and store Information about Tables/Layers without details [SpatialiteLayerType]
    if ( getSniffMinimal() )
    {
      // Determine the amount of LayerTypeTables that exist, if valid
      if ( getSniffLayerMetadata() )
      {
        // The amount of Layers for each Layer-Type is determined, if valid.
        if ( getSniffReadLayers() )
        {
          // Everything is still valid.
          if ( getSniffType() == SpatialiteDbInfo::SniffMinimal )
          {
            prepare();
            if ( getQSqliteHandle() )
            {
              getQSqliteHandle()->setStatus();
            }
            // -- ---------------------------------- --
            // For where only the list of what is available is needed
            // - SpatialiteDbInfo::SniffMinimal must run before GetDbLayersInfo can be called.
            // -> SpatialiteDbInfo must be created with either SpatialiteDbInfo::SniffMinimal or SpatialiteDbInfo::SniffLoadLayers
            // --> not with SpatialiteDbInfo::SniffDatabaseType
            // -- ---------------------------------- --
            QgsDebugMsgLevel( QString( "-I-> retrieveSniffMinimal[%23,%1] -SniffMinimal- IsValid[%17] IsSpatialite[%18] IsGdalOgr[%19] dbLayersCount[%20]  VectorLayers[%2,%3] SpatialTables[%4] SpatialViews[%5] VirtualShapes[%6] VectorCoverages[%7] VectorCoveragesStyles[%8] RasterCoverages[%9] RasterCoveragesStyles[%10] RasterLite1[%11] TopologyExport[%12] FdoOgr[%13] GeoPackage[%14] MBTiles[%15] NonSpatial[%16] isDbSpatialiteActive[%21] isDbRasterLite2Active[%22] FileName[%24]" ).arg( dbSpatialMetadataString() ).arg( dbVectorLayersCount() ).arg( mHasLegacyGeometryLayers ).arg( dbSpatialTablesLayersCount() ).arg( dbSpatialViewsLayersCount() ).arg( dbVirtualShapesLayersCount() ).arg( dbVectorCoveragesLayersCount() ).arg( dbVectorStylesViewsCount() ).arg( dbRasterCoveragesLayersCount() ).arg( dbRasterStylesViewCount() ).arg( dbRasterLite1LayersCount() ).arg( dbTopologyExportLayersCount() ).arg( dbFdoOgrLayersCount() ).arg( dbGeoPackageLayersCount() ).arg( dbMBTilesLayersCount() ).arg( dbNonSpatialTablesCount() ).arg( isDbValid() ).arg( isDbSpatialite() ).arg( isDbGdalOgr() ).arg( dbLayersCount() ).arg( isDbSpatialiteActive() ).arg( isDbRasterLite2Active() ).arg( getSniffTypeString() ).arg( getFileName() ), 7 );
            // -- ---------------------------------- --
            return isDbValid();
            // -- ---------------------------------- --
          }
        }
      }
    }
  }
  return isDbValid();
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::getSniffDatabaseType
//-----------------------------------------------------------------
bool SpatialiteDbInfo::getSniffDatabaseType()
{
  // -- ---------------------------------- --
  // -- This must be a Sqlite3 Database
  // -- ---------------------------------- --
  if ( isDbValid() )
  {
    int i_rc = 0;
    // Note: travis reports: error: use of undeclared identifier 'sqlite3_db_readonly'
    i_rc = sqlite3_db_readonly( dbSqliteHandle(), "main" );
    switch ( i_rc )
    {
      case 0:
        mReadOnly = false;
        break;
      case 1:
        mReadOnly = true;
        break;
      case -1:
      default:
        setDatabaseInvalid( getQSqliteHandle()->dbPath(), QString( "getSpatialiteLayerInfo: sqlite3_db_readonly failed." ) );
        break;
    }
  }
  return isDbValid();
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::getSniffMinimal
//-----------------------------------------------------------------
// Precondition: is a Sqlite3-Container [isValid]
// Determin which Sqlite3-Container it is:
// The Sql-Query will search for specific TABLE/Field combinations
// - based on these results, the Container Type can be determined
// -> -1 : does not exist ; 0 does exist
// --> will check the set '0' TABLEs to determin if empty or filled
//  The existance of some TABLEs will be used to build later queries
//-----------------------------------------------------------------
// RasterLite1: 'layer_statistics' must exist
// RasterLite2: 'raster_coverages' must exist
// GeoPackage :  'gpkg_contents' must exist
// MBTiles : contains at least 2 TABLES called 'metadata','tiles','images','map'
// TABLE 'geometry_columns':
// SpatialiteLegacy (< 4) : contains a column 'spatial_index_enabled' and 'coord_dimension' as TEXT
// Spatialite4  (>= 4): contains a column 'spatial_index_enabled' and 'coord_dimension' as INTEGER
// FdoOgr : contains a column 'geometry_format'
//-----------------------------------------------------------------
bool SpatialiteDbInfo::getSniffMinimal()
{
  //---------------------------------------------------------------
  if ( isDbValid() )
  {
    bool bHitValid = false;
    sqlite3_stmt *stmt = nullptr;
    QString sql = QStringLiteral( "SELECT " );
    // (SELECT CASE WHEN (Exists(SELECT rootpage FROM sqlite_master WHERE (name = 'raster_coverages')) = 0) THEN -1 ELSE 0  END),
    sql += QStringLiteral( "(SELECT CASE WHEN (Exists(SELECT rootpage FROM sqlite_master WHERE (name = 'vector_coverages')) = 0) THEN -1 ELSE 0  END)" );
    sql += QStringLiteral( " AS vector_coverages," ); // 00
    sql += QStringLiteral( "(SELECT CASE WHEN (Exists(SELECT rootpage FROM sqlite_master WHERE (name = 'raster_coverages')) = 0) THEN -1 ELSE 0  END)" );
    sql += QStringLiteral( " AS layout_rasterlite2," ); // 01
    sql += QStringLiteral( "(SELECT CASE WHEN (Exists(SELECT rootpage FROM sqlite_master WHERE (name = 'topologies')) = 0) THEN -1 ELSE 0  END)" );
    sql += QStringLiteral( " AS layout_topologies," ); // 02
    sql += QStringLiteral( "(SELECT CASE WHEN (Exists(SELECT rootpage FROM sqlite_master WHERE (name = 'data_licenses')) = 0) THEN -1 ELSE 5  END)" );
    sql += QStringLiteral( " AS layout_spatialite45," ); // 03
    sql += QStringLiteral( "(SELECT CASE WHEN (Exists(SELECT rootpage FROM sqlite_master WHERE (name = 'layer_statistics')) = 0) THEN -1 ELSE 0  END)" );
    sql += QStringLiteral( " AS layer_statistics," ); // 04
    sql += QStringLiteral( "(SELECT CASE WHEN (Exists(SELECT rootpage FROM sqlite_master WHERE (name = 'vector_layers')) = 0) THEN -1 ELSE 0  END)" );
    sql += QStringLiteral( " AS vector_layers," ); // 05
    sql += QStringLiteral( "(SELECT CASE WHEN (Exists(SELECT rootpage FROM sqlite_master WHERE (name = 'SE_vector_styles_view')) = 0) THEN -1 ELSE 0  END)" );
    sql += QStringLiteral( " AS vector_styles," ); // 06
    sql += QStringLiteral( "(SELECT CASE WHEN (Exists(SELECT rootpage FROM sqlite_master WHERE (name = 'SE_raster_styles_view')) = 0) THEN -1 ELSE 0  END)" );
    sql += QStringLiteral( " AS raster_styles," ); // 07
    //---------------------------------------------------------------
    // Note: Both Spatialite and GdalFdoOgr have a 'geometry_columns' TABLE
    // - The Spatialite < 4 version contains a column 'spatial_index_enabled' and 'coord_dimension' as TEXT
    // - The Spatialite >= 4 version contains a column 'spatial_index_enabled' and 'coord_dimension' as INTEGER
    // - The GdalFdoOgr version contains a column 'geometry_format',
    //---------------------------------------------------------------
    sql += QStringLiteral( "(SELECT CASE WHEN (Exists(SELECT rootpage FROM sqlite_master WHERE ((name = 'geometry_columns') AND (sql LIKE '%coord_dimension TEXT%') AND (sql LIKE '%spatial_index_enabled%'))) = 0) THEN -1 ELSE 0  END)" );
    sql += QStringLiteral( " AS layout_spatialite_legacy," ); // 08
    sql += QStringLiteral( "(SELECT CASE WHEN (Exists(SELECT rootpage FROM sqlite_master WHERE ((name = 'geometry_columns') AND (sql LIKE '%coord_dimension INTEGER%') AND (sql LIKE '%spatial_index_enabled%'))) = 0) THEN -1 ELSE 0  END)" );
    sql += QStringLiteral( " AS layout_spatialite," ); // 09
    sql += QStringLiteral( "(SELECT CASE WHEN (Exists(SELECT rootpage FROM sqlite_master WHERE ((name = 'geometry_columns') AND (sql LIKE '%geometry_format%'))) = 0) THEN -1 ELSE 0  END)" );
    sql += QStringLiteral( " AS layout_fdoogr," ); // 10
    sql += QStringLiteral( "(SELECT CASE WHEN (Exists(SELECT tbl_name FROM sqlite_master WHERE (type = 'table' AND tbl_name='gpkg_contents')) = 0) THEN -1 ELSE 0  END)" );
    sql += QStringLiteral( " AS layout_geopackage," ); // 11
    sql += QStringLiteral( "(SELECT CASE WHEN (Exists(SELECT tbl_name FROM sqlite_master WHERE (type = 'view' AND tbl_name='vector_layers_auth')) = 0) THEN -1 ELSE 0  END)" );
    sql += QStringLiteral( " AS vector_layers_auth," ); // 12
    // MBTiles: at least 2 specific tables ['metadata' alone can often be found elsewhere]
    sql += QStringLiteral( "(SELECT CASE WHEN (SELECT (count(tbl_name) > 1) FROM sqlite_master WHERE ((type = 'table' OR type = 'view') AND (tbl_name IN ('metadata','tiles','images','map'))) = 0) THEN -1 ELSE 0 END)" );
    sql += QStringLiteral( " AS layout_mbtiles" ); // 13
    int i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
    // QgsDebugMsgLevel( QString( "-I-> SpatialiteDbInfo::getSniffMinimals: i_rc[%1]  sql[%2]" ).arg( i_rc ).arg( sql ), 7 );
    if ( i_rc == SQLITE_OK )
    {
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
        {
          mHasVectorCoveragesTables = sqlite3_column_int( stmt, 0 ); // vector_coverages
          if ( mHasVectorCoveragesTables == 0 )
          {
            bHitValid = true;
          }
        }
        if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
        {
          mHasRasterCoveragesTables = sqlite3_column_int( stmt, 1 ); // layout_rasterlite2
          if ( mHasRasterCoveragesTables == 0 )
          {
            bHitValid = true;
          }
        }
        if ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL )
        {
          mHasTopologyExportTables = sqlite3_column_int( stmt, 2 ); // layout_topologies
          if ( mHasTopologyExportTables == 0 )
          {
            bHitValid = true;
          }
        }
        if ( sqlite3_column_type( stmt, 3 ) != SQLITE_NULL )
        {
          if ( sqlite3_column_int( stmt, 3 ) == 5 ) // layout_spatialite45
          {
            setSpatialMetadata( ( int )Spatialite45 );
            bHitValid = true;
          }
        }
        if ( sqlite3_column_type( stmt, 4 ) != SQLITE_NULL )
        {
          mHasRasterLite1Tables = sqlite3_column_int( stmt, 4 ); // layer_statistics
          if ( mHasRasterLite1Tables == 0 )
          {
            bHitValid = true;
          }
        }
        if ( sqlite3_column_type( stmt, 5 ) != SQLITE_NULL )
        {
          mHasVectorLayers = sqlite3_column_int( stmt, 5 ); // vector_layers
          if ( mHasVectorLayers == 0 )
          {
            bHitValid = true;
          }
        }
        if ( sqlite3_column_type( stmt, 6 ) != SQLITE_NULL )
        {
          mHasVectorStylesView = sqlite3_column_int( stmt, 6 ); // vector_styles
          if ( mHasVectorStylesView == 0 )
          {
            bHitValid = true;
          }
        }
        if ( sqlite3_column_type( stmt, 7 ) != SQLITE_NULL )
        {
          mHasRasterStylesView = sqlite3_column_int( stmt, 7 ); // raster_styles
          if ( mHasRasterStylesView == 0 )
          {
            bHitValid = true;
          }
        }
        if ( sqlite3_column_type( stmt, 8 ) != SQLITE_NULL )
        {
          if ( mHasVectorLayers < 0 )
          {
            mHasLegacyGeometryLayers = sqlite3_column_int( stmt, 8 ); // layout_spatialite_legacy
            mHasVectorLayers = mHasLegacyGeometryLayers;
            if ( mHasLegacyGeometryLayers == 0 )
            {
              bHitValid = true;
              setSpatialMetadata( ( int )SpatialiteLegacy );
            }
          }
        }
        if ( sqlite3_column_type( stmt, 9 ) != SQLITE_NULL )
        {
          if ( mHasVectorLayers < 0 )
          {
            mHasVectorLayers = sqlite3_column_int( stmt, 9 ); // layout_spatialite
            if ( mHasVectorLayers == 0 )
            {
              bHitValid = true;
              if ( mSpatialMetadata  != Spatialite45 )
              {
                setSpatialMetadata( ( int )Spatialite40 );
              }
            }
          }
        }
        if ( sqlite3_column_type( stmt, 10 ) != SQLITE_NULL )
        {
          mHasFdoOgrTables = sqlite3_column_int( stmt, 10 ); // layout_fdoogr
          if ( mHasFdoOgrTables == 0 )
          {
            bHitValid = true;
            setSpatialMetadata( ( int )SpatialiteFdoOgr );
          }
        }
        if ( sqlite3_column_type( stmt, 11 ) != SQLITE_NULL )
        {
          mHasGeoPackageTables = sqlite3_column_int( stmt, 11 ); // layout_geopackage
          if ( mHasGeoPackageTables == 0 )
          {
            bHitValid = true;
            setSpatialMetadata( ( int )SpatialiteGpkg );
          }
        }
        if ( sqlite3_column_type( stmt, 12 ) != SQLITE_NULL )
        {
          mHasVectorLayersAuth = sqlite3_column_int( stmt, 12 ); // vector_layers_auth
          if ( mHasVectorLayersAuth == 0 )
          {
            bHitValid = true;
          }
        }
        if ( sqlite3_column_type( stmt, 13 ) != SQLITE_NULL )
        {
          mHasMBTilesTables = sqlite3_column_int( stmt, 13 ); // layout_mbtiles
          if ( mHasMBTilesTables == 0 )
          {
            bHitValid = true;
            // This will call checkMBTiles and set everything properly
            setSpatialMetadata( ( int )SpatialUnknown );
          }
        }
      }
      sqlite3_finalize( stmt );
    }
    else
    {
      if ( i_rc == 5 )
      {
        mIsDbLocked = true;
      }
      // getSniffMinimal: rc=[5,The database file is locked]
      QgsDebugMsgLevel( QString( "getSniffMinimal: rc=[%1,%2] FileName[%3] sql[%4]" ).arg( i_rc ).arg( QgsSqliteHandle::get_sqlite3_result_code_string( i_rc ) ).arg( getDatabaseFileName() ).arg( sql ), 7 );
    }
    if ( !bHitValid )
    {
      // Nothing we know was found, prevent everthing else from running [most likly non supported Sqlite3-Container]
      mIsDbValid = false;
      QgsDebugMsgLevel( QString( "-I-> getSniffMinimal -HasNot- IsValid[%11] VectorLayers[%1] RasterLite2Rasters[%2] RasterLite2Vectors[%3] RasterLite1[%4] TopologyExport[%5] FdoOgr[%6] GeoPackage[%7] MBTiles[%8] VectorsStyles[%9] RasterStyles[%10]  FileName[%12]" ).arg( mHasVectorLayers ).arg( mHasRasterCoveragesTables ).arg( mHasVectorCoveragesTables ).arg( mHasRasterLite1Tables ).arg( mHasTopologyExportTables ).arg( mHasFdoOgrTables ).arg( mHasGeoPackageTables ).arg( mHasMBTilesTables ).arg( mHasVectorStylesView ).arg( mHasRasterStylesView ).arg( isDbValid() ).arg( getDatabaseFileName() ), 7 );
    }
#if 0
    QgsDebugMsgLevel( QString( "-I-> getSniffSniffMinima(%1,%23) IsValid[%17,%25] IsSpatialite[%18] IsGdalOgr[%19] dbLayersCount[%20] -Has- VectorLayers[%2,%3] SpatialTables[%4] SpatialViews[%5] VirtualShapes[%6] VectorCoverages[%7] VectorCoveragesStyles[%8] RasterCoverages[%9] RasterCoveragesStyles[%10] RasterLite1[%11] TopologyExport[%12] FdoOgr[%13] GeoPackage[%14] MBTiles[%15] NonSpatial[%16] isDbSpatialiteActive[%21] isDbRasterLite2Active[%22] FileName[%24]" ).arg( dbSpatialMetadataString() ).arg( dbVectorLayersCount() ).arg( mHasLegacyGeometryLayers ).arg( dbSpatialTablesLayersCount() ).arg( dbSpatialViewsLayersCount() ).arg( dbVirtualShapesLayersCount() ).arg( dbVectorCoveragesLayersCount() ).arg( dbVectorStylesViewsCount() ).arg( dbRasterCoveragesLayersCount() ).arg( dbRasterStylesViewCount() ).arg( dbRasterLite1LayersCount() ).arg( dbTopologyExportLayersCount() ).arg( dbFdoOgrLayersCount() ).arg( dbGeoPackageLayersCount() ).arg( dbMBTilesLayersCount() ).arg( dbNonSpatialTablesCount() ).arg( isDbValid() ).arg( isDbSpatialite() ).arg( isDbGdalOgr() ).arg( dbLayersCount() ).arg( isDbSpatialiteActive() ).arg( isDbRasterLite2Active() ).arg( getSniffTypeString() ).arg( getFileName() ).arg( bHitValid ), 7 );
#endif
  }
  return isDbValid();
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::getSniffLayerMetadata
//-----------------------------------------------------------------
// Precondition: at least 1 of the TABLE types == 0 [isValid]
// The Sql-Query will count the amount of found entries
// - at least 1 TABLE entry must be > 0
// -> otherwise considered invalid
//-----------------------------------------------------------------
bool SpatialiteDbInfo::getSniffLayerMetadata()
{
  //---------------------------------------------------------------
  if ( isDbValid() )
  {
    QString sql;
    int i_rc;
    sqlite3_stmt *stmt = nullptr;
    bool bHitValid = false;
    //---------------------------------------------------------------
    if ( mHasRasterLite1Tables == 0 )
    {
      sql = QStringLiteral( "SELECT " );
      // truemarble-4km.sqlite
      sql += QStringLiteral( "(SELECT count(raster_layer) FROM 'layer_statistics' WHERE (raster_layer=1))," );
      sql += QStringLiteral( "(SELECT group_concat(table_name,',') FROM 'layer_statistics' WHERE ((raster_layer=0) AND (table_name LIKE '%_metadata')))" );
      int i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
      if ( i_rc == SQLITE_OK )
      {
        QString rl1Tables;
        while ( sqlite3_step( stmt ) == SQLITE_ROW )
        {
          if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
          {
            mHasRasterLite1Tables = sqlite3_column_int( stmt, 0 );
          }
          if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
          {
            rl1Tables = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
          }
        }
        sqlite3_finalize( stmt );
        if ( !rl1Tables.isEmpty() )
        {
          QStringList rl1Parts = rl1Tables.split( ',', QString::SkipEmptyParts );
          for ( int i = 0; i < rl1Parts.size(); i++ )
          {
            // TrueMarble_metadata,srtm_metadata
            sql = QStringLiteral( "SELECT count(id) FROM '%1' WHERE ((id=0) AND (source_name = 'raster metadata'))" ).arg( rl1Parts.at( i ) );
            i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
            if ( i_rc == SQLITE_OK )
            {
              while ( sqlite3_step( stmt ) == SQLITE_ROW )
              {
                if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
                {
                  if ( sqlite3_column_int( stmt, 0 ) == 1 )
                  {
                    mHasRasterLite1Tables++;
                  }
                }
              }
              sqlite3_finalize( stmt );
            }
          }
        }
      }
      if ( mHasRasterLite1Tables > 0 )
      {
        bHitValid = true;
      }
    }
    //---------------------------------------------------------------
    if ( mHasVectorLayers == 0 )
    {
      sql = QStringLiteral( "SELECT count(table_name) FROM 'vector_layers'" );
      if ( mHasLegacyGeometryLayers == 0 )
      {
        sql = QStringLiteral( "SELECT count(f_table_name) FROM 'geometry_columns'" );
      }
      i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
      if ( i_rc == SQLITE_OK )
      {
        while ( sqlite3_step( stmt ) == SQLITE_ROW )
        {
          if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
          {
            mHasVectorLayers = sqlite3_column_int( stmt, 0 );
            if ( mHasLegacyGeometryLayers == 0 )
            {
              mHasLegacyGeometryLayers = mHasVectorLayers;
            }
          }
        }
        sqlite3_finalize( stmt );
      }
      if ( mHasVectorLayers > 0 )
      {
        bHitValid = true;
      }
    }
    //---------------------------------------------------------------
    if ( mHasTopologyExportTables == 0 )
    {
      sql = QStringLiteral( "SELECT count(topology_name) FROM 'topologies'" );
      i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
      if ( i_rc == SQLITE_OK )
      {
        while ( sqlite3_step( stmt ) == SQLITE_ROW )
        {
          if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
          {
            mHasTopologyExportTables = sqlite3_column_int( stmt, 0 );
          }
        }
        sqlite3_finalize( stmt );
      }
      if ( mHasTopologyExportTables > 0 )
      {
        bHitValid = true;
      }
    }
    //---------------------------------------------------------------
    if ( mHasVectorCoveragesTables == 0 )
    {
      sql = QStringLiteral( "SELECT count(coverage_name) FROM 'vector_coverages'" );
      i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
      if ( i_rc == SQLITE_OK )
      {
        while ( sqlite3_step( stmt ) == SQLITE_ROW )
        {
          if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
          {
            mHasVectorCoveragesTables = sqlite3_column_int( stmt, 0 );
          }
        }
        sqlite3_finalize( stmt );
      }
      if ( mHasVectorCoveragesTables > 0 )
      {
        bHitValid = true;
      }
    }
    //---------------------------------------------------------------
    if ( mHasRasterCoveragesTables == 0 )
    {
      sql = QStringLiteral( "SELECT count(coverage_name) FROM 'raster_coverages'" );
      i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
      if ( i_rc == SQLITE_OK )
      {
        while ( sqlite3_step( stmt ) == SQLITE_ROW )
        {
          if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
          {
            mHasRasterCoveragesTables = sqlite3_column_int( stmt, 0 );
          }
        }
        sqlite3_finalize( stmt );
      }
      if ( mHasRasterCoveragesTables > 0 )
      {
        bHitValid = true;
      }
    }
    //---------------------------------------------------------------
    if ( mHasVectorStylesView == 0 )
    {
      // 'SE_vector_styled_layers_view' contains spatialite specific functions [XB_GetTitle, XB_GetAbstract, XB_IsSchemaValidated, XB_GetSchemaURI]
      // - without a Spatialite connecton the title and Abstract cannot be retrieved
      // 'SE_vector_styled_layers' contains the same, but without the spatialite specific commands
      // 'SE_raster_styles_view' lists all Styles, registered or not
      sql = QStringLiteral( "SELECT count(coverage_name) FROM 'SE_vector_styled_layers'" );
      i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
      if ( i_rc == SQLITE_OK )
      {
        while ( sqlite3_step( stmt ) == SQLITE_ROW )
        {
          if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
          {
            mHasVectorStylesView = sqlite3_column_int( stmt, 0 );
          }
        }
        sqlite3_finalize( stmt );
      }
      if ( mHasVectorStylesView > 0 )
      {
        bHitValid = true;
      }
      // QgsDebugMsgLevel( QString( "-I-> getSniffLayerMetadata rc[%1,%3] sql[%2] " ).arg( i_rc ).arg( sql ).arg(sqlite3_errmsg( dbSqliteHandle() )), 4 );
    }
    //---------------------------------------------------------------
    if ( mHasRasterStylesView == 0 )
    {
      // 'SE_raster_styled_layers_view' contains spatialite specific functions [XB_GetTitle, XB_GetAbstract, XB_IsSchemaValidated, XB_GetSchemaURI]
      // - without a Spatialite connecton the title and Abstract cannot be retrieved
      // 'SE_raster_styled_layers' contains the same, but without the spatialite specific commands
      // 'SE_raster_styles_view' lists all Styles, registered or not
      sql = QStringLiteral( "SELECT count(coverage_name) FROM 'SE_raster_styled_layers'" );
      i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
      if ( i_rc == SQLITE_OK )
      {
        while ( sqlite3_step( stmt ) == SQLITE_ROW )
        {
          if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
          {
            mHasRasterStylesView = sqlite3_column_int( stmt, 0 );
          }
        }
        sqlite3_finalize( stmt );
      }
      if ( mHasRasterStylesView > 0 )
      {
        bHitValid = true;
      }
    }
    //---------------------------------------------------------------
    if ( mHasGeoPackageTables == 0 )
    {
      sql = QStringLiteral( "SELECT count(table_name) FROM 'gpkg_contents'" );
      i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
      if ( i_rc == SQLITE_OK )
      {
        while ( sqlite3_step( stmt ) == SQLITE_ROW )
        {
          if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
          {
            mHasGeoPackageTables = sqlite3_column_int( stmt, 0 );
          }
        }
        sqlite3_finalize( stmt );
      }
      if ( mHasGeoPackageTables > 0 )
      {
        bHitValid = true;
      }
    }
    //---------------------------------------------------------------
    if ( mHasFdoOgrTables == 0 )
    {
      //---------------------------------------------------------------
      // Note: Both Spatialite and GdalFdoOgr have a 'geometry_columns' TABLE
      // - The GdalFdoOgr version contains a column 'geometry_format', The Spatialite version does not.
      //---------------------------------------------------------------
      sql = QStringLiteral( "SELECT count(geometry_format) FROM 'geometry_columns'" );
      i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
      if ( i_rc == SQLITE_OK )
      {
        while ( sqlite3_step( stmt ) == SQLITE_ROW )
        {
          if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
          {
            mHasFdoOgrTables = sqlite3_column_int( stmt, 0 );
          }
        }
        sqlite3_finalize( stmt );
      }
      if ( mHasFdoOgrTables > 0 )
      {
        bHitValid = true;
      }
    }
    if ( mHasMBTilesTables >= 0 )
    {
      bHitValid = true;
    }
    //---------------------------------------------------------------
    if ( !bHitValid )
    {
      if ( i_rc == 5 )
      {
        mIsDbLocked = true;
      }
      // This may be a valid Database, but is empty
      mIsEmpty = true;
      QgsDebugMsgLevel( QString( "-W-> getSniffLayerMetadata(%1,%23) IsValid[%17] IsEmpty[%25] IsSpatialite[%18] IsGdalOgr[%19] dbLayersCount[%20] -Has- VectorLayers[%2,%3] SpatialTables[%4] SpatialViews[%5] VirtualShapes[%6] VectorCoverages[%7] VectorCoveragesStyles[%8] RasterCoverages[%9] RasterCoveragesStyles[%10] RasterLite1[%11] TopologyExport[%12] FdoOgr[%13] GeoPackage[%14] MBTiles[%15] NonSpatial[%16] isDbSpatialiteActive[%21] isDbRasterLite2Active[%22] FileName[%24]" ).arg( dbSpatialMetadataString() ).arg( dbVectorLayersCount() ).arg( mHasLegacyGeometryLayers ).arg( dbSpatialTablesLayersCount() ).arg( dbSpatialViewsLayersCount() ).arg( dbVirtualShapesLayersCount() ).arg( dbVectorCoveragesLayersCount() ).arg( dbVectorStylesViewsCount() ).arg( dbRasterCoveragesLayersCount() ).arg( dbRasterStylesViewCount() ).arg( dbRasterLite1LayersCount() ).arg( dbTopologyExportLayersCount() ).arg( dbFdoOgrLayersCount() ).arg( dbGeoPackageLayersCount() ).arg( dbMBTilesLayersCount() ).arg( dbNonSpatialTablesCount() ).arg( isDbValid() ).arg( isDbSpatialite() ).arg( isDbGdalOgr() ).arg( dbLayersCount() ).arg( isDbSpatialiteActive() ).arg( isDbRasterLite2Active() ).arg( getSniffTypeString() ).arg( getFileName() ).arg( isDbEmpty() ), 7 );
    }
    else
    {
      readNonSpatialTables();
      //---------------------------------------------------------------
#if 1
      QgsDebugMsgLevel( QString( "-I-> getSniffLayerMetadata(%1,%23) IsValid[%17] IsSpatialite[%18] IsGdalOgr[%19] dbLayersCount[%20] -Has- VectorLayers[%2,%3] SpatialTables[%4] SpatialViews[%5] VirtualShapes[%6] VectorCoverages[%7] VectorCoveragesStyles[%8] RasterCoverages[%9] RasterCoveragesStyles[%10] RasterLite1[%11] TopologyExport[%12] FdoOgr[%13] GeoPackage[%14] MBTiles[%15] NonSpatial[%16] isDbSpatialiteActive[%21] isDbRasterLite2Active[%22] FileName[%24]" ).arg( dbSpatialMetadataString() ).arg( dbVectorLayersCount() ).arg( mHasLegacyGeometryLayers ).arg( dbSpatialTablesLayersCount() ).arg( dbSpatialViewsLayersCount() ).arg( dbVirtualShapesLayersCount() ).arg( dbVectorCoveragesLayersCount() ).arg( dbVectorStylesViewsCount() ).arg( dbRasterCoveragesLayersCount() ).arg( dbRasterStylesViewCount() ).arg( dbRasterLite1LayersCount() ).arg( dbTopologyExportLayersCount() ).arg( dbFdoOgrLayersCount() ).arg( dbGeoPackageLayersCount() ).arg( dbMBTilesLayersCount() ).arg( dbNonSpatialTablesCount() ).arg( isDbValid() ).arg( isDbSpatialite() ).arg( isDbGdalOgr() ).arg( dbLayersCount() ).arg( isDbSpatialiteActive() ).arg( isDbRasterLite2Active() ).arg( getSniffTypeString() ).arg( getFileName() ), 7 );
#endif
    }
  }
  return isDbValid();
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::getSniffReadLayers
//-----------------------------------------------------------------
bool SpatialiteDbInfo::getSniffReadLayers()
{
  //---------------------------------------------------------------
  if ( isDbValid() )
  {
    if ( mHasVectorLayers > 0 )
    {
      readVectorLayers();
      // This must be done here since it may not be a Spatialite Database [such as MBTiles]
      if ( mHasVectorLayers <= 0 )
      {
        setDatabaseInvalid( getQSqliteHandle()->dbPath(), QString( "getSpatialiteLayerInfo: No Vector Layers found[%1]." ).arg( mHasVectorLayers ) );
      }
      else
      {
        mTotalCountLayers += dbVectorLayersCount();
      }
    }
    if ( ( mHasVectorCoveragesTables > 0 ) || ( mHasRasterCoveragesTables > 0 ) )
    {
      readVectorRasterCoverages();
      mTotalCountLayers += dbRasterCoveragesLayersCount();
    }
    if ( ( mHasVectorStylesView > 0 ) || ( mHasRasterStylesView > 0 ) )
    {
      // Spatialite driver support is needed to retreive the xml-documents
      // - if there is no connection, this will be done later on demand in getDbStyleNamedLayerElement
      bool bTestStylesForQgis = true;
      readVectorRasterStyles( bTestStylesForQgis );
    }
    if ( mHasTopologyExportTables > 0 )
    {
      readTopologyLayers();
      mTotalCountLayers += dbTopologyExportLayersCount();
    }
    if ( mHasRasterLite1Tables > 0 )
    {
      readRasterLite1Layers();
      mTotalCountLayers += dbRasterLite1LayersCount();
    }
    if ( mHasMBTilesTables > 0 )
    {
      readMBTilesLayers();
      mTotalCountLayers += dbMBTilesLayersCount();
    }
    if ( mHasGeoPackageTables > 0 )
    {
      readGeoPackageLayers();
      mTotalCountLayers += dbGeoPackageLayersCount();
    }
    if ( mHasFdoOgrTables > 0 )
    {
      readFdoOgrLayers();
      mTotalCountLayers += dbFdoOgrLayersCount();
    }

    // -- ---------------------------------- --
    // do only on creation, not when (possibly) adding to collection.
    mDbLayers.clear();

    // -- ---------------------------------- --
#if 1
    QgsDebugMsgLevel( QString( "-I-> getSniffReadLayers(%1,%23) IsValid[%17] IsSpatialite[%18] IsGdalOgr[%19] dbLayersCount[%20]  -Has- VectorLayers[%2,%3] SpatialTables[%4] SpatialViews[%5] VirtualShapes[%6] VectorCoverages[%7] VectorCoveragesStyles[%8] RasterCoverages[%9] RasterCoveragesStyles[%10] RasterLite1[%11] TopologyExport[%12] FdoOgr[%13] GeoPackage[%14] MBTiles[%15] NonSpatial[%16] isDbSpatialiteActive[%21] isDbRasterLite2Active[%22] FileName[%24]" ).arg( dbSpatialMetadataString() ).arg( dbVectorLayersCount() ).arg( mHasLegacyGeometryLayers ).arg( dbSpatialTablesLayersCount() ).arg( dbSpatialViewsLayersCount() ).arg( dbVirtualShapesLayersCount() ).arg( dbVectorCoveragesLayersCount() ).arg( dbVectorStylesViewsCount() ).arg( dbRasterCoveragesLayersCount() ).arg( dbRasterStylesViewCount() ).arg( dbRasterLite1LayersCount() ).arg( dbTopologyExportLayersCount() ).arg( dbFdoOgrLayersCount() ).arg( dbGeoPackageLayersCount() ).arg( dbMBTilesLayersCount() ).arg( dbNonSpatialTablesCount() ).arg( isDbValid() ).arg( isDbSpatialite() ).arg( isDbGdalOgr() ).arg( dbLayersCount() ).arg( isDbSpatialiteActive() ).arg( isDbRasterLite2Active() ).arg( getSniffTypeString() ).arg( getFileName() ), 7 );
#endif
  }
  return isDbValid();
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::getSniffLoadLayers
//-----------------------------------------------------------------
bool SpatialiteDbInfo::getSniffLoadLayers( QString sLayerName )
{
  //---------------------------------------------------------------
  if ( isDbValid() )
  {
    bool bFoundLayerName = false;
    GetDbLayersInfo( sLayerName, bFoundLayerName );
  }
  return isDbValid();
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::setSniffType
//-----------------------------------------------------------------
void SpatialiteDbInfo::setSniffType( SpatialSniff sniffType )
{
  if ( ( sniffType == SpatialiteDbInfo::SniffLoadLayers ) && ( mSniffType != sniffType ) && ( dbLayersCount() > 0 ) )
  {
    // SpatialiteDbInfo may has been created with SniffMinimal, but now we want all the LayerInformation
    mSniffType = sniffType;
    // -read all Layers now, instead of 1 by 1 which would be slower [by large Databases noticeable]
    getSniffLoadLayers( QString::null );
  }
  else
  {
    mSniffType = sniffType;
  }
}

//-----------------------------------------------------------------
// SpatialiteDbInfo::readVectorLayers
//-----------------------------------------------------------------
bool SpatialiteDbInfo::readVectorLayers()
{
  bool bRc = false;
  //---------------------------------------------------------------
  if ( isDbValid() )
  {
    mVectorLayers.clear();
    mVectorLayersTypes.clear();
    sqlite3_stmt *stmt = nullptr;
    mHasSpatialTables = 0;
    mHasSpatialViews = 0;
    mHasVirtualShapes = 0;
    QString sql = QStringLiteral( "SELECT layer_type,table_name||'('||geometry_column||')' AS layer_name, geometry_type,coord_dimension, srid, spatial_index_enabled FROM vector_layers ORDER BY layer_type, table_name,geometry_column" );
    if ( mHasVectorLayersAuth > -1 )
    {
      // sandro [Alessandro Furieri] 2017-11-27 11:40:36
      // There is an "hidden" column on the following tables: "geometry_columns_auth", "views_geometry_columns_auth", "virts_geometry_columns_auth" and "vector_layers_auth";
      // as far as I can remember all the above "*_auth" tables were required by the now definitively abandoned GaiaGis tool.
      // for the moment only for SpatialTable [SpatialView and VirtualShape will return NULL]
      // sql = QStringLiteral( "SELECT a.layer_type,a.table_name||'('||a.geometry_column||')' AS layer_name, a.geometry_type,a.coord_dimension, a.srid, a.spatial_index_enabled, b.hidden FROM vector_layers AS a LEFT JOIN vector_layers_auth AS b ON (Upper(a.table_name) = Upper(b.table_name) AND Upper(a.geometry_column) = Upper(b.geometry_column))  ORDER BY a.layer_type, a.table_name,a.geometry_column" );
    }
    if ( mHasLegacyGeometryLayers > 0 )
    {
      // vector_layers (since 4.0.0) is a VIEW, simulate it where it does not exist
      sql = QStringLiteral( "SELECT 'SpatialTable' AS layer_type, f_table_name||'('||f_geometry_column||')' AS layer_name, type AS geometry_type, coord_dimension AS coord_dimension, srid AS srid, spatial_index_enabled FROM geometry_columns UNION " );
      sql += QStringLiteral( "SELECT 'SpatialView' AS layer_type, a.view_name||'('||a.view_geometry||')' AS layer_name, b.type AS geometry_type, b.coord_dimension AS coord_dimension, b.srid AS srid,  b.spatial_index_enabled AS spatial_index_enabled FROM views_geometry_columns AS a " );
      sql += QStringLiteral( "LEFT JOIN geometry_columns AS b ON (Upper(a.f_table_name) = Upper(b.f_table_name) AND Upper(a.f_geometry_column) = Upper(b.f_geometry_column)) UNION " );
      sql += QStringLiteral( "SELECT 'VirtualShape' AS layer_type, virt_name||'('||virt_geometry||')' AS layer_name, type AS geometry_type, '' AS coord_dimension, srid AS srid, 0 AS spatial_index_enabled  FROM virts_geometry_columns  ORDER BY layer_type, layer_name" );
    }
    int  i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      QString sLayerType;
      QString sLayerName;
      QString sGeometryType;
      QString sDataSourceUri;
      QString sLayerInfo;
      int iSpatialIndex = GAIA_VECTOR_UNKNOWN;
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
        {
          sLayerType = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
          SpatialiteDbInfo::SpatialiteLayerType layerType = SpatialiteDbInfo::SpatialiteLayerTypeFromName( sLayerType );
          if ( ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL ) &&
               ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL ) &&
               ( sqlite3_column_type( stmt, 3 ) != SQLITE_NULL ) &&
               ( sqlite3_column_type( stmt, 4 ) != SQLITE_NULL ) )
          {
            bool bValid = true;
            sLayerName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
            if ( dbSpatialMetadata() != SpatialiteDbInfo::SpatialiteLegacy )
            {
              sGeometryType = QgsWkbTypes::displayString( SpatialiteDbLayer::GetGeometryType( sqlite3_column_int( stmt, 2 ), sqlite3_column_int( stmt, 3 ) ) );
            }
            else
            {
              // Translate Spatialite-Legacy 'geometry_type' and 'coord_dimension' to present day version
              sGeometryType = QgsWkbTypes::displayString( SpatialiteDbLayer::GetGeometryTypeLegacy( QString::fromUtf8( ( const char * )sqlite3_column_text( stmt, 2 ) ), QString::fromUtf8( ( const char * )sqlite3_column_text( stmt, 3 ) ) ) );
            }
            int iSrid = sqlite3_column_int( stmt, 4 );
            iSpatialIndex = GAIA_VECTOR_UNKNOWN;
            if ( sqlite3_column_type( stmt, 5 ) != SQLITE_NULL )
            {
              switch ( sqlite3_column_type( stmt, 5 ) )
              {
                case 0:
                  iSpatialIndex = GAIA_SPATIAL_INDEX_NONE;
                  break;
                case 1:
                  iSpatialIndex = GAIA_SPATIAL_INDEX_RTREE;
                  break;
                case 2:
                  iSpatialIndex = GAIA_SPATIAL_INDEX_MBRCACHE;
                  break;
                default:
                  iSpatialIndex = GAIA_VECTOR_UNKNOWN;
                  break;
              }
            }
            int iIsHidden = 0;
            if ( sqlite3_column_type( stmt, 6 ) != SQLITE_NULL )
            {
              // for the moment only for SpatialTable [SpatialView and VirtualShape will return NULL]
              iIsHidden = sqlite3_column_type( stmt, 6 );
            }
            switch ( layerType )
            {
              case SpatialiteDbInfo::SpatialTable:
                mHasSpatialTables++;
                break;
              case SpatialiteDbInfo::SpatialView:
              {
                mHasSpatialViews++;
                QString sTableName = QString::null;
                QString sGeometryColumn = QString::null;
                //----------------------------------------------------------
                // Extract TableName/GeometryColumn from sent 'table_name' or 'table_name(field_name)' from LayerName
                SpatialiteDbInfo::parseLayerName( sLayerName, sTableName, sGeometryColumn );
                //----------------------------------------------------------
                if ( sTableName.contains( "_" ) )
                {
                  QStringList sa_list_name = sTableName.split( "_" );
                  if ( sa_list_name.size() >= 2 )
                  {
                    // add last '_' to insure that 'pg_stat_borders' does not include 'pg_stadteile_*'
                    QString sListGroupName = QString( "%1_%2_" ).arg( sa_list_name.at( 0 ) ).arg( sa_list_name.at( 1 ) );
                    // Make sure that a Layer-Name is only added once
                    if ( !mMapGroupNames.contains( sLayerName ) )
                    {
                      mListGroupNames.append( sListGroupName.toLower() );
                      mMapGroupNames.insert( sLayerName, 0 );
                    }
                  }
                }
              }
              break;
              case SpatialiteDbInfo::VirtualShape:
                mHasVirtualShapes++;
                break;
              default:
                // Unexpected value, do not add
                bValid = false;
                break;
            }
            if ( bValid )
            {
              createDbLayerInfoUri( sLayerInfo, sDataSourceUri, sLayerName, layerType, sGeometryType, iSrid, iSpatialIndex, iIsHidden );
              mVectorLayers.insert( sLayerName, sLayerInfo );
              mVectorLayersTypes.insert( sLayerName, sLayerType );
            }
          }
        }

      }
      sqlite3_finalize( stmt );
    }
    mHasVectorLayers = mVectorLayers.count();
    if ( mHasVectorLayers > 0 )
    {
      bRc = true;
      if ( mListGroupNames.size() > 0 )
      {
        // mListGroupNames:will be filled filled with (possible) non-unique values
        // mListGroupNames: all single entries are removed ; List contains unique values.
        prepareGroupLayers();
      }
    }
  }
  //---------------------------------------------------------------
  return bRc;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::readVectorRasterCoverages
//-----------------------------------------------------------------
bool SpatialiteDbInfo::readVectorRasterCoverages()
{
  bool bRc = false;
  //---------------------------------------------------------------
  if ( isDbValid() )
  {
    sqlite3_stmt *stmt = nullptr;
    QString sql;
    int i_rc;
    if ( mHasVectorCoveragesTables > 0 )
    {
      mVectorCoveragesLayers.clear();
      mVectorCoveragesLayersExtent.clear();
      QMap<QString, QString> vectorCoveragesLayerName;
      QString subQueryLicense = "(SELECT name FROM 'data_licenses' WHERE id=(SELECT license FROM 'vector_coverages'))";
      // 'vector_coverages_ref_sys' will allways contain the original srid (native_srid=1) and optional alternative srid's (native_srid=0)
      // - the extent is gven both with the native (extent_min/max/x/y) and Wsg84 (geo_min/max/x/y)
      QString subQuerySrid = "(SELECT srid FROM 'vector_coverages_ref_sys' WHERE ((vector_coverages_ref_sys.coverage_name=vector_coverages.coverage_name) AND (native_srid=1)))";
      sql = QStringLiteral( "SELECT coverage_name,title,abstract,copyright,%1 AS name_license,%2 AS srid,f_table_name, f_geometry_column, view_name, view_geometry FROM 'vector_coverages' ORDER BY coverage_name" ).arg( subQueryLicense ).arg( subQuerySrid );
      i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
      // QgsDebugMsgLevel( QString( "-I-> SpatialiteDbInfo::readVectorRasterCoverages[vector] rc[%1] sql[%2]" ).arg( i_rc ).arg( sql ), 4 );
      if ( i_rc == SQLITE_OK )
      {
        QString sLayerType = SpatialiteDbInfo::SpatialiteLayerTypeName( SpatialiteDbInfo::RasterLite2Vector );
        while ( sqlite3_step( stmt ) == SQLITE_ROW )
        {
          if ( ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL ) &&
               ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL ) )
          {
            // Note: CoverageName may be different than the LayerName [table_name(geoemtry_name)]
            QString sCoverageName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
            QString sTitle = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
            QString sAbstract = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 2 ) );
            QString sCopyright = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 3 ) );
            QString sLicense = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 4 ) );
            int iSrid = sqlite3_column_int( stmt, 5 );
            QString sLayerName;
            QString sTableName;
            QString sGeometryName;
            // SpatialTable: view_name/view_geometry are Null
            if ( ( sqlite3_column_type( stmt, 6 ) != SQLITE_NULL ) &&
                 ( sqlite3_column_type( stmt, 7 ) != SQLITE_NULL ) )
            {
              sTableName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 6 ) );
              sGeometryName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 7 ) );
            }
            // SpatialView: f_table_name,/f_geometry_column are Null
            if ( ( sqlite3_column_type( stmt, 8 ) != SQLITE_NULL ) &&
                 ( sqlite3_column_type( stmt, 9 ) != SQLITE_NULL ) )
            {
              sTableName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 8 ) );
              sGeometryName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 9 ) );
            }
            sLayerName = QString( "%1(%2)" ).arg( sTableName ).arg( sGeometryName );
            if ( !sLayerName.isEmpty() )
            {
              // Use the Euro-Character to parse, hoping it will not be used in the Title/Abstract Text
              QString sValue = QString( "%2%1%3%1%4%1%5%1%6%1%7%1%8" ).arg( SpatialiteDbInfo::ParseSeparatorCoverage ).arg( sLayerType ).arg( sCoverageName ).arg( sTitle ).arg( sAbstract ).arg( sCopyright ).arg( sLicense ).arg( iSrid );
              mVectorCoveragesLayers.insert( sLayerName, sValue );
              vectorCoveragesLayerName.insert( sCoverageName, sLayerName );
            }
          }
        }
        sqlite3_finalize( stmt );
        sql = QString( "SELECT coverage_name,native_srid||'%1'||srid||'%1'||extent_minx||'%1'||extent_miny||'%1'||extent_maxx||'%1'||extent_maxy" ).arg( SpatialiteDbInfo::ParseSeparatorCoverage );
        sql += QString( "||'%1'||geo_minx||'%1'||geo_miny||'%1'||geo_maxx||'%1'||geo_maxy AS extent_values FROM 'vector_coverages_ref_sys' ORDER BY coverage_name,native_srid DESC" ).arg( SpatialiteDbInfo::ParseSeparatorCoverage );
        // srid;is_nativ;extent_minx;extent_miny;extent_maxx;extent_maxy;geo_minx;geo_miny;geo_maxx;geo_maxy
        i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
        // QgsDebugMsgLevel( QString( "-I-> SpatialiteDbInfo::readVectorRasterCoverages[vector] rc[%1] sql[%2]" ).arg( i_rc ).arg( sql ), 4 );
        if ( i_rc == SQLITE_OK )
        {
          while ( sqlite3_step( stmt ) == SQLITE_ROW )
          {
            if ( ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL ) &&
                 ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL ) )
            {
              // Note: CoverageName may be different than the LayerName [table_name(geoemtry_name)]
              QString sCoverageName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
              QString sValue = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
              QString sLayerName = vectorCoveragesLayerName.value( sCoverageName );
              if ( sLayerName.size() > 0 )
              {
                mVectorCoveragesLayersExtent.insert( sLayerName, sValue );
                // QgsDebugMsgLevel( QString( "-I-> SpatialiteDbInfo::readVectorRasterCoverages[vector] LayerName[%1] ExtentValue[%2]" ).arg( sLayerName ).arg( sValue ), 4 );
              }
            }
          }
          sqlite3_finalize( stmt );
        }
      }
      mHasVectorCoveragesTables = mVectorCoveragesLayers.count();
    }
    if ( mHasRasterCoveragesTables > 0 )
    {
      mRasterCoveragesLayers.clear();
      mRasterCoveragesLayersExtent.clear();
      QString subQueryLicense = "(SELECT name FROM 'data_licenses' WHERE id=(SELECT license FROM 'raster_coverages'))";
      sql = QStringLiteral( "SELECT coverage_name,title,abstract,copyright,%1 AS name_license,srid FROM 'raster_coverages' ORDER BY coverage_name" ).arg( subQueryLicense );
      i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
      // QgsDebugMsgLevel( QString( "-I-> SpatialiteDbInfo::readVectorRasterCoverages[raster] rc[%1] sql[%2]" ).arg( i_rc ).arg( sql ), 4 );
      if ( i_rc == SQLITE_OK )
      {
        QString sDataSourceUri;
        QString sLayerInfo;
        QString sLayerType = SpatialiteDbInfo::SpatialiteLayerTypeName( SpatialiteDbInfo::RasterLite2Raster );
        while ( sqlite3_step( stmt ) == SQLITE_ROW )
        {
          if ( ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL ) &&
               ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL ) )
          {
            QString sLayerName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
            QString sTitle = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
            QString sAbstract = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 2 ) );
            QString sCopyright = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 3 ) );
            QString sLicense = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 4 ) );
            int iSrid = sqlite3_column_int( stmt, 5 );
            // Use the Euro-Character to parse, hoping it will not be used in the Title/Abstract Text
            QString sValue = QString( "%2%1%3%1%4%1%5%1%6%1%7%1%8" ).arg( SpatialiteDbInfo::ParseSeparatorCoverage ).arg( sLayerType ).arg( sLayerName ).arg( sTitle ).arg( sAbstract ).arg( sCopyright ).arg( sLicense ).arg( iSrid );
            // QgsDebugMsgLevel( QString( "-I-> SpatialiteDbInfo::readVectorRasterCoverages[raster] LayerName[%1] Value[%2]" ).arg( sLayerName ).arg( sValue ), 4 );
            mRasterCoveragesLayers.insert( sLayerName, sValue );
            //--------------------------------
            QMap<QString, QString> layerTypes;
            // Note: Key must be unique
            layerTypes.insert( QString( "%1_sections(%2)" ).arg( sLayerName ).arg( "geometry" ), QStringLiteral( "RasterLite2-Sections" ) );
            layerTypes.insert( QString( "%1_tiles(%2)" ).arg( sLayerName ).arg( "geometry" ), QStringLiteral( "RasterLite2-Tiles" ) );
            // Remove (and delete) the RasterLite2 Admin tables, which should not be shown [returns amount deleted (not needed)]
            i_rc = removeAdminTables( layerTypes );
          }
        }
        sqlite3_finalize( stmt );
        sql = QString( "SELECT coverage_name,native_srid||'%1'||srid||'%1'||extent_minx||'%1'||extent_miny||'%1'||extent_maxx||'%1'||extent_maxy" ).arg( SpatialiteDbInfo::ParseSeparatorCoverage );
        sql += QString( "||'%1'||geo_minx||'%1'||geo_miny||'%1'||geo_maxx||'%1'||geo_maxy AS extent_values FROM 'raster_coverages_ref_sys' ORDER BY coverage_name,native_srid DESC" ).arg( SpatialiteDbInfo::ParseSeparatorCoverage );
        // srid;is_nativ;extent_minx;extent_miny;extent_maxx;extent_maxy;geo_minx;geo_miny;geo_maxx;geo_maxy
        i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
        // QgsDebugMsgLevel( QString( "-I-> SpatialiteDbInfo::readVectorRasterCoverages[vector] rc[%1] sql[%2]" ).arg( i_rc ).arg( sql ), 4 );
        if ( i_rc == SQLITE_OK )
        {
          while ( sqlite3_step( stmt ) == SQLITE_ROW )
          {
            if ( ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL ) &&
                 ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL ) )
            {
              QString sCoverageName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
              QString sValue = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
              if ( sCoverageName.size() > 0 )
              {
                mRasterCoveragesLayersExtent.insert( sCoverageName, sValue );
              }
            }
          }
          sqlite3_finalize( stmt );
        }
      }
      mHasRasterCoveragesTables = mRasterCoveragesLayers.count();
      if ( mHasRasterCoveragesTables > 0 )
      {
        if ( isDbSpatialite() )
        {
          mIsRasterLite2 = true;
        }
        bRc = true;
        if ( !mHasGdalRasterLite2Driver )
        {

          sql = QStringLiteral( "SQLite" ); // RasterLite2
          GDALDriverH rl2GdalDriver = GDALGetDriverByName( sql.toLocal8Bit().constData() );
          if ( rl2GdalDriver )
          {
            mHasGdalRasterLite2Driver = true;
          }
        }
      }
    }
  }
  //---------------------------------------------------------------
  return bRc;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::readVectorRasterStyles
//-----------------------------------------------------------------
int SpatialiteDbInfo::readVectorRasterStyles( bool bTestStylesForQgis )
{
  mVectorStyleInfo.clear();
  mRasterStyleInfo.clear();
  mVectorStyleData.clear();
  mRasterStyleData.clear();
  mLayerNamesStyles.clear();
  //---------------------------------------------------------------
  if ( isDbValid() )
  {
    sqlite3_stmt *stmt = nullptr;
    QString sql;
    int i_rc;
    // true=Retrieve only those Styles being used  (ignore the others)
    // This view contains spatialite specfic functions [XB_GetTitle, XB_GetAbstract, XB_IsSchemaValidated, XB_GetSchemaURI]
    // - these Views
    QString sVectorStyleTableName = "SE_vector_styled_layers_view";
    QString sRasterStyleTableName = "SE_raster_styled_layers_view";
    QString sFieldName = "coverage_name";
    QString sTestStylesForQgis = QString( "validation_not_checked" );
    if ( mHasVectorStylesView > 0 )
    {
      // first: build list of registered CoverageNames with style_id and fill mLayerNamesStyles
      sql = QStringLiteral( "SELECT coverage_name, style_id FROM 'SE_vector_styled_layers' ORDER BY coverage_name" );
      i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
      if ( i_rc == SQLITE_OK )
      {
        while ( sqlite3_step( stmt ) == SQLITE_ROW )
        {
          if ( ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL ) &&
               ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL ) )
          {
            QString sCoverageName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
            int iStyleId = sqlite3_column_int( stmt, 1 );
            mLayerNamesStyles.insert( sCoverageName, iStyleId );
          }
        }
        sqlite3_finalize( stmt );
      }
      if ( isDbSpatialiteActive() )
      {
        // SELECT DISTINCT name,XB_GetDocument(style,1),title,abstract FROM 'SE_vector_styled_layers_view' ORDER BY name
        mHasVectorStylesView = 0;
        sql = QStringLiteral( "SELECT DISTINCT style_id, name,XB_GetDocument(style,1),title,abstract FROM '%1' ORDER BY name" ).arg( sVectorStyleTableName );
        i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
        if ( i_rc == SQLITE_OK )
        {
          while ( sqlite3_step( stmt ) == SQLITE_ROW )
          {
            if ( ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL ) &&
                 ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )  &&
                 ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL ) )
            {
              int iStyleId = sqlite3_column_int( stmt, 0 );
              QString sStyleName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
              QString sStyleXml = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 2 ) );
              if ( sStyleXml.size() > 0 )
              {
                mVectorStyleData.insert( iStyleId, sStyleXml );
              }
              QString sStyleTitle;
              QString sStyleAbstract;
              if ( bTestStylesForQgis )
              {
                int i_rc_test = 1;
#if 1
                getDbStyleNamedLayerElement( SpatialiteDbInfo::StyleVector, iStyleId, sTestStylesForQgis, &i_rc_test, QString() );
#else
                // Note: with the file-name, the Style will be written to disk for debugging
                getDbStyleNamedLayerElement( SpatialiteDbInfo::StyleVector, iStyleId, sTestStylesForQgis, &i_rc_test, QString( QString( "qgis.sld.%1.sld" ).arg( sStyleName ) ).arg( sStyleName ) );
#endif
                if ( i_rc_test != 0 )
                {
                  QgsDebugMsgLevel( QString( "[Style] TestStylesForQgis for [%1] failed rc=%2 error[%3]" ).arg( sStyleName ).arg( i_rc_test ).arg( sTestStylesForQgis ), 3 );
                  sTestStylesForQgis = QString( "validation_not_checked" );
                  mVectorStyleData.remove( iStyleId );
                }
                else
                {
                  sTestStylesForQgis = QString( "valid" );
                }
              }
              if ( ( sqlite3_column_type( stmt, 3 ) != SQLITE_NULL ) &&
                   ( sqlite3_column_type( stmt, 4 ) != SQLITE_NULL ) )
              {
                sStyleTitle = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 3 ) );
                sStyleAbstract = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 4 ) );
              }
              // Use the Euro-Character to parse between, hoping it will not be used in the Title/Abstract Text
              sStyleXml = QString( "%2%1%3%1%4%1%5" ).arg( SpatialiteDbInfo::ParseSeparatorCoverage ).arg( sStyleName ).arg( sStyleTitle ).arg( sStyleAbstract ).arg( sTestStylesForQgis );
              mVectorStyleInfo.insert( iStyleId, sStyleXml );
            }
          }
          sqlite3_finalize( stmt );
        }
        mHasVectorStylesView = mVectorStyleInfo.count();
      }
    }
    if ( mHasRasterStylesView > 0 )
    {
      // first: build list of registered CoverageNames with style_id and fill mLayerNamesStyles
      sql = QStringLiteral( "SELECT coverage_name, style_id FROM 'SE_raster_styled_layers' ORDER BY coverage_name" );
      i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
      if ( i_rc == SQLITE_OK )
      {
        while ( sqlite3_step( stmt ) == SQLITE_ROW )
        {
          if ( ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL ) &&
               ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL ) )
          {
            QString sCoverageName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
            int iStyleId = sqlite3_column_int( stmt, 1 );
            mLayerNamesStyles.insert( sCoverageName, iStyleId );
          }
        }
        sqlite3_finalize( stmt );
      }
      if ( isDbSpatialiteActive() )
      {
        mHasRasterStylesView = 0;
        int i_count_data_insert = 0;
        sql = QStringLiteral( "SELECT DISTINCT style_id,name,XB_GetDocument(style,1),title,abstract FROM'%1' ORDER BY name" ).arg( sRasterStyleTableName );
        i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
        if ( i_rc == SQLITE_OK )
        {
          while ( sqlite3_step( stmt ) == SQLITE_ROW )
          {
            if ( ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL ) &&
                 ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )  &&
                 ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL ) )
            {
              int iStyleId = sqlite3_column_int( stmt, 0 );
              QString sStyleName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
              QString sStyleXml = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 2 ) );
              if ( sStyleXml.size() > 0 )
              {
                mRasterStyleInfo.insert( iStyleId, sStyleXml );
                i_count_data_insert++;
              }
              QString sStyleTitle;
              QString sStyleAbstract;
              if ( ( sqlite3_column_type( stmt, 3 ) != SQLITE_NULL ) &&
                   ( sqlite3_column_type( stmt, 4 ) != SQLITE_NULL ) )
              {
                sStyleTitle = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 3 ) );
                sStyleAbstract = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 4 ) );
              }
              // Use the Euro-Character to parse, hoping it will not be used in the Title/Abstract Text
              sStyleXml = QString( "%2%1%3%1%4%1%5" ).arg( SpatialiteDbInfo::ParseSeparatorCoverage ).arg( sStyleName ).arg( sStyleTitle ).arg( sStyleAbstract ).arg( sTestStylesForQgis );
              mRasterStyleInfo.insert( iStyleId, sStyleXml );
              mHasRasterStylesView++;
            }
          }
          sqlite3_finalize( stmt );
        }
      }
    }
  }
  //---------------------------------------------------------------
  return mLayerNamesStyles.count();
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::readTopologyLayers
//-----------------------------------------------------------------
bool SpatialiteDbInfo::readRasterLite1Layers()
{
  bool bRc = false;
  //----------------------------------------------------------------
  if ( isDbValid() )
  {
    mRasterLite1Layers.clear();
    sqlite3_stmt *stmt = nullptr;
    QString sql = QStringLiteral( "SELECT " );
    sql += QStringLiteral( "(SELECT group_concat(table_name,',') FROM 'layer_statistics' WHERE (raster_layer=1))," );
    sql += QStringLiteral( "(SELECT group_concat(replace(table_name,'_metadata',''),',') FROM 'layer_statistics' WHERE ((raster_layer=0) AND (table_name LIKE '%_metadata')))" );
    int i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      QString rl1Tables;
      QStringList saRL1Tables;
      int iSpatialIndex = GAIA_VECTOR_UNKNOWN;
      int iIsHidden = 0;
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
        {
          // truemarble-4km.sqlite: truemarble-4km.sqlite
          rl1Tables = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
          if ( !rl1Tables.isEmpty() )
          {
            saRL1Tables.append( rl1Tables.split( "," ) );
          }
        }
        if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
        {
          // ItalyRail.atlas: TrueMarble_metadata,srtm_metadata
          rl1Tables = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
          if ( !rl1Tables.isEmpty() )
          {
            saRL1Tables.append( rl1Tables.split( "," ) );
          }
        }
      }
      sqlite3_finalize( stmt );
      if ( saRL1Tables.size() )
      {
        for ( int i = 0; i < saRL1Tables.size(); i++ )
        {
          QString sRasterLite1Name = saRL1Tables.at( i );
          // TrueMarble_metadata,srtm_metadata
          sql = QStringLiteral( "SELECT " );
          sql += QStringLiteral( "(SELECT Exists(SELECT id FROM '%1' WHERE ((id=0) AND (source_name = 'raster metadata'))))," ).arg( QString( "%1_metadata" ).arg( sRasterLite1Name ) );
          sql += QStringLiteral( "(SELECT Exists(SELECT id FROM '%1' WHERE (id=1)))," ).arg( QString( "%1_rasters" ).arg( sRasterLite1Name ) );
          sql += QStringLiteral( "(SELECT srid FROM 'geometry_columns' WHERE (f_table_name = '%1_metadata'))" ).arg( sRasterLite1Name );
          // SELECT srid FROM 'geometry_columns' WHERE (f_table_name = 'TrueMarble_metadata')
          i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
          QgsDebugMsgLevel( QString( "-I-> SpatialiteDbInfo::readRasterLite1Layers: mRasterLite1Layers i_rc[%1]  sql[%2]" ).arg( i_rc ).arg( sql ), 7 );
          if ( i_rc == SQLITE_OK )
          {
            QString sDataSourceUri;
            QString sLayerInfo;
            QString sLayerType = SpatialiteDbInfo::SpatialiteLayerTypeName( SpatialiteDbInfo::RasterLite1 );
            while ( sqlite3_step( stmt ) == SQLITE_ROW )
            {
              if ( ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL ) &&
                   ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL ) &&
                   ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL ) )
              {
                if ( ( sqlite3_column_int( stmt, 0 ) == 1 ) && ( sqlite3_column_int( stmt, 1 ) == 1 ) )
                {
                  // The internal-Data needed for RasterLite1 exist and could therefore be valid
                  int iSrid = sqlite3_column_int( stmt, 2 );
                  createDbLayerInfoUri( sLayerInfo, sDataSourceUri, sRasterLite1Name, SpatialiteDbInfo::RasterLite1, sLayerType, iSrid, iSpatialIndex, iIsHidden );
                  // -I-> SpatialiteDbInfo::readRasterLite1Layers: mRasterLite1Layers Key[middle_earth_1418SR]  LayerInfo[RasterLite1;3035;gdal;RasterLite1;-1;0]
                  mRasterLite1Layers.insert( sRasterLite1Name, sLayerInfo );
                  //--------------------------------
                  // Note: Key must be unique
                  QMap<QString, QString> layerTypes;
                  layerTypes.insert( QString( "%1_metadata(%2)" ).arg( sRasterLite1Name ).arg( "geometry" ), QStringLiteral( "RasterLite1-Metadata" ) );
                  // Remove (and delete) the RasterLite1 Admin tables, which should not be shown [returns amount deleted (not needed)]
                  i_rc = removeAdminTables( layerTypes );
                  // QgsDebugMsgLevel( QString( "-I-> SpatialiteDbInfo::readRasterLite1Layers: mRasterLite1Layers Key[%1]  LayerInfo[%2]" ).arg( sRasterLite1Name ).arg( sLayerInfo ), 7 );
                }
              }
            }
            sqlite3_finalize( stmt );
          }
        }
      }
    }
    mHasRasterLite1Tables = mRasterLite1Layers.count();
    if ( mHasRasterLite1Tables > 0 )
    {
      bRc = true;
      sql = QStringLiteral( "RasterLite" );
      GDALDriverH rl1GdalDriver = GDALGetDriverByName( sql.toLocal8Bit().constData() );
      if ( rl1GdalDriver )
      {
        mHasGdalRasterLite1Driver = true;
        mIsGdalOgr = true;
      }
    }
  }
  //---------------------------------------------------------------
  return bRc;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::readTopologyLayers
//-----------------------------------------------------------------
bool SpatialiteDbInfo::readTopologyLayers()
{
  bool bRc = false;
  //---------------------------------------------------------------
  if ( isDbValid() )
  {
    mTopologyNames.clear();
    mTopologyExportLayers.clear();
    sqlite3_stmt *stmt = nullptr;
    QString sql = QStringLiteral( "SELECT topology_name,srid FROM 'topologies'" );
    int i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      int iSrid = -1;
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
        {
          if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
          {
            mTopologyNames.append( QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) ) );
            iSrid = sqlite3_column_int( stmt, 1 );
          }
        }
      }
      sqlite3_finalize( stmt );
      if ( mTopologyNames.size() > 0 )
      {
        for ( int i = 0; i < mTopologyNames.size(); i++ )
        {
          QString sTopologyName = mTopologyNames.at( i );
          QString sDataSourceUri;
          QString sLayerInfo;
          int iSpatialIndex = GAIA_VECTOR_UNKNOWN;
          int iIsHidden = 0;
          sql = QString( "SELECT topolayer_name FROM '%1'" ).arg( "%1_topolayers" ).arg( sTopologyName );
          i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
          if ( i_rc == SQLITE_OK )
          {
            while ( sqlite3_step( stmt ) == SQLITE_ROW )
            {
              if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
              {
                QString sTopoLayerName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
                QString sLayerName = QString( "%1(%2)" ).arg( sTopologyName ).arg( sTopoLayerName );
                // TODO: retrieve the geometry-type from the SpatialTable
                QString sGeometryType;
                createDbLayerInfoUri( sLayerInfo, sDataSourceUri, sLayerName, SpatialiteDbInfo::TopologyExport, sGeometryType, iSrid, iSpatialIndex, iIsHidden );
                mTopologyExportLayers.insert( sLayerName, sLayerInfo );
              }
            }
            sqlite3_finalize( stmt );
          }
        }
        mHasTopologyExportTables = mTopologyExportLayers.count();
        if ( mHasTopologyExportTables > 0 )
        {
          bRc = true;
        }
      }
    }
  }
  //---------------------------------------------------------------
  return bRc;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::readMagicHeaderFromFile
//-----------------------------------------------------------------
SpatialiteDbInfo::MimeType SpatialiteDbInfo::readMagicHeaderFromFile( QString sDatabaseFileName )
{
  MimeType mimeType = MimeUnknown;
  //---------------------------------------------------------------
  // https://en.wikipedia.org/wiki/List_of_file_signatures
  // https://gist.github.com/navinpai/983632
  // https://www.iana.org/assignments/media-types/media-types.xhtml
  //---------------------------------------------------------------
  QFileInfo file_info( sDatabaseFileName );
  if ( file_info.exists() )
  {
    if ( file_info.isFile() )
    {
      mimeType = MimeFile;
      if ( file_info.isReadable() )
      {
        if ( file_info.isExecutable() )
        {
          mimeType = MimeFileExecutable;
        }
        else
        {
          QFile read_file( sDatabaseFileName );
          if ( read_file.open( QIODevice::ReadOnly ) )
          {
            int iMaxStringLength = 128;
            // Read the maximum amount needed, so that the file is read only once.
            QByteArray baFilePeek = read_file.peek( iMaxStringLength );
            read_file.close();
            QString sFilePeekString = QString( "%1" ).arg( QString::fromStdString( baFilePeek.toStdString() ) );
            QString sSuffix = file_info.suffix().toLower();
            //---------------------------------------------------------------
            // 'Tricky the comparison of byte-data combinded with characters through the use of latin is.'
            //-- undocumented quote from Yoda
            //---------------------------------------------------------------
            if ( baFilePeek.startsWith( QByteArray( "SQLite format 3" ) ) )
            {
              mimeType = MimeSqlite3;
              return mimeType;
            }
            else if ( baFilePeek.startsWith( QByteArray( "%PDF" ) ) )
            {
              mimeType = MimePdf;
            }
            else if ( baFilePeek.startsWith( QByteArray( "GIF87a" ) ) )
            {
              mimeType = MimeGif87a;
            }
            else if ( baFilePeek.startsWith( QByteArray( "GIF89a" ) ) )
            {
              mimeType = MimeGif89a;
            }
            else if ( ( baFilePeek.startsWith( QByteArray( "II*" ) ) ) || ( baFilePeek.startsWith( QByteArray( "MM.*" ) ) ) )
            {
              mimeType = MimeTiff;
            }
            else if ( ( baFilePeek.startsWith( QByteArray::fromHex( "FFD8FFD9" ) ) ) ||
                      ( baFilePeek.startsWith( QByteArray::fromHex( "FFD8FFE0" ) ) ) )
            {
              mimeType = MimeJpeg;
            }
            else if ( baFilePeek.startsWith( QByteArray::fromHex( "0000000C6A5020200D0A870A" ) ) )
            {
              mimeType = MimeJp2;  // 12 byte string: X'0000 000C 6A50 2020  0D0A 870A'
            }
            else if ( baFilePeek.startsWith( QByteArray::fromHex( "89504E470D0A1A0A" ) ) )
            {
              mimeType = MimePng; // \x89PNG\x0d\x0a\x1a\x0a
            }
            else if ( baFilePeek.startsWith( QByteArray( "RIFF " ) ) )
            {
              if ( baFilePeek.contains( QByteArray( "AVI" ) ) )
              {
                mimeType = MimeAvi;
              }
              if ( baFilePeek.contains( QByteArray( "WAVE" ) ) )
              {
                mimeType = MimeWav;
              }
              if ( baFilePeek.contains( QByteArray( "WEBP" ) ) )
              {
                mimeType = MimeWebp;
              }
            }
            else if ( ( baFilePeek.startsWith( QByteArray::fromHex( "504B0304" ) ) ) ||
                      ( baFilePeek.startsWith( QByteArray::fromHex( "504B0506" ) ) ) )
            {
              mimeType = MimeZip;
              if ( sSuffix == QStringLiteral( "kmz" ) )
              {
                mimeType = MimeKmz;
              }
            }
            else if ( baFilePeek.startsWith( QByteArray::fromHex( "425A68" ) ) )
            {
              mimeType = MimeBz2;
            }
            else if ( ( baFilePeek.startsWith( QByteArray::fromHex( "1F9D" ) ) ) ||
                      ( baFilePeek.startsWith( QByteArray::fromHex( "1FA0" ) ) ) )
            {
              mimeType = MimeTar;
            }
            else if ( ( baFilePeek.startsWith( QByteArray::fromHex( "526172211A0700" ) ) ) ||
                      ( baFilePeek.startsWith( QByteArray::fromHex( "526172211A070100" ) ) ) )
            {
              mimeType = MimeRar;
            }
            else if ( baFilePeek.startsWith( QByteArray::fromHex( "377ABCAF271C" ) ) )
            {
              mimeType = Mime7z;
            }
            else if ( baFilePeek.startsWith( QByteArray::fromHex( "78617221" ) ) )
            {
              mimeType = MimeXar;
            }
            else if ( baFilePeek.startsWith( QByteArray( "<?xml" ) ) )
            {
              mimeType = MimeXml;
              if ( baFilePeek.contains( QByteArray( "<kml" ) ) )
              {
                mimeType = MimeKml;
              }
              if ( baFilePeek.contains( QByteArray( "<svg" ) ) )
              {
                mimeType = MimeSvg;
              }
            }
            else if ( baFilePeek.startsWith( QByteArray( "<PAMDataset>" ) ) )
            {
              mimeType = MimeGdalPam;
            }
            else if ( baFilePeek.startsWith( QByteArray( "<GMLFeatureClassList>" ) ) )
            {
              mimeType = MimeGdalGml;
            }
            else if ( baFilePeek.startsWith( QByteArray::fromHex( "7B5C72746631" ) ) )
            {
              mimeType = MimeRtf;
            }
            else if ( baFilePeek.startsWith( QByteArray::fromHex( "00000100" ) ) )
            {
              mimeType = MimeIco;
            }
            else if ( baFilePeek.startsWith( QByteArray::fromHex( "7F454C46" ) ) )
            {
              mimeType = MimeExeUnix;
            }
            else if ( baFilePeek.startsWith( QByteArray( "#!/bin/" ) ) )
            {
              mimeType = MimeScripts;
              if ( ( baFilePeek.contains( QByteArray( "#!/bin/bash" ) ) ) ||
                   ( baFilePeek.contains( QByteArray( "#!/bin/sh" ) ) ) )
              {
                mimeType = MimeBash;
              }
              if ( baFilePeek.contains( QByteArray( "#!/bin/python" ) ) )
              {
                mimeType = MimePython;
              }
            }
            else if ( baFilePeek.startsWith( QByteArray( "<!DOCTYPE " ) ) )
            {
              mimeType = MimeHtmlDTD;
              if ( baFilePeek.contains( QByteArray( "<qgis projectname" ) ) )
              {
                mimeType = MimeQGisProject;
              }
            }
            else if ( baFilePeek.startsWith( QByteArray( "** This file contains an SQLite 2" ) ) )
            {
              mimeType = MimeSqlite2;   // '** This file contains an SQLite 2.1 database **'
            }
            else if ( ( sSuffix == QStringLiteral( "txt" ) ) ||
                      ( sSuffix == QStringLiteral( "c" ) ) ||
                      ( sSuffix == QStringLiteral( "h" ) ) ||
                      ( sSuffix == QStringLiteral( "cpp" ) ) ||
                      ( sSuffix == QStringLiteral( "log" ) ) ||
                      ( sSuffix == QStringLiteral( "pid" ) ) ||
                      ( sSuffix == QStringLiteral( "sql" ) ) )
            {
              mimeType = MimeTxt;
            }
            //---------------------------------------------------------------
            if ( mimeType == MimeFile )
            {
              // QgsDebugMsgLevel( QString( "-W-> SpatialiteDbInfo::readMagicHeaderFromFile[%2,%3,%4]: FilePeekString[%1}" ).arg( sFilePeekString ).arg(file_info.suffix()).arg(file_info.completeSuffix()).arg(mimeType), 4 );
            }
          }
        }
      }
      else
      {
        mimeType = MimeFileNotReadable;
      }
    }
    else
    {
      if ( file_info.isDir() )
      {
        mimeType = MimeDirectory;
        if ( file_info.isSymLink() )
        {
          mimeType = MimeDirectorySymLink;
        }
        if ( file_info.isExecutable() )
        {
          mimeType = MimeDirectoryExecutable;
        }
        else
        {
          if ( !file_info.isReadable() )
          {
            mimeType = MimeDirectoryNotReadable;
          }
        }
      }
    }
  }
  else
  {
    mimeType = MimeNotExists;
  }
  // Network connection?
  //---------------------------------------------------------------
  return mimeType;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::createDatabase
//-----------------------------------------------------------------
bool SpatialiteDbInfo::createDatabase()
{
  bool bRc = false;
  if ( mSqliteHandle )
  {
    return bRc;
  }
  bool bInitSpatialite = true;
  //---------------------------------------------------------------
  int open_flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX;
  if ( QgsSqliteHandle::sqlite3_open_v2( mDatabaseFileName.toUtf8().constData(), &mSqliteHandle, open_flags, nullptr, bInitSpatialite )  == SQLITE_OK )
  {
    sqlite3_stmt *stmt = nullptr;
    // retrieve and set spatialite version - the spatialite connection exists , which must be closed when this function finishes
    QString sql = QStringLiteral( "SELECT spatialite_version()" );
    int i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
        {
          mSpatialiteVersionInfo = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
          QString sVersionInfo = mSpatialiteVersionInfo;
          sVersionInfo.remove( QRegExp( QString::fromUtf8( "[-`~!@#$%^&*()_—+=|:;<>«»,?/{a-zA-Z}\'\"\\[\\]\\\\]" ) ) );
          QStringList spatialiteParts = sVersionInfo.split( ' ', QString::SkipEmptyParts );
          // Get major, minor and revision version
          QStringList spatialiteVersionParts = spatialiteParts[0].split( '.', QString::SkipEmptyParts );
          if ( spatialiteVersionParts.size() == 3 )
          {
            mSpatialiteVersionMajor = spatialiteVersionParts[0].toInt();
            mSpatialiteVersionMinor = spatialiteVersionParts[1].toInt();
            mSpatialiteVersionRevision = spatialiteVersionParts[2].toInt();
          }
        }
      }
      sqlite3_finalize( stmt );
    }
  }
  if ( ( mSpatialiteVersionMajor > 2 ) || ( ( mSpatialiteVersionMajor == 2 ) && ( mSpatialiteVersionMinor >= 4 ) ) )
  {
    // Only continue wth a usable Spatialite-Connection
    switch ( mDbCreateOption )
    {
      case SpatialMetadata::Spatialite40:
      case SpatialMetadata::Spatialite45:
        bRc = createDatabaseSpatialite();
        break;
      case SpatialMetadata::SpatialiteGpkg:
        bRc = createDatabaseGeoPackage();
        break;
      case SpatialMetadata::SpatialiteMBTiles:
        bRc = createDatabaseMBTiles();
        break;
      case SpatialMetadata::SpatialiteLegacy:
      case SpatialMetadata::SpatialUnknown:
      case SpatialMetadata::SpatialiteFdoOgr:
      default:
        break;
    }
    //---------------------------------------------------------------
    if ( bRc )
    {
      bRc = false;
      if ( SpatialiteDbInfo::readMagicHeaderFromFile( mDatabaseFileName ) ==  SpatialiteDbInfo::MimeSqlite3 )
      {
        mIsSqlite3 = true;
        mMimeType = SpatialiteDbInfo::MimeSqlite3;
        // The File exists and is a Sqlite3 container
        QFileInfo file_info( mDatabaseFileName );
        mDatabaseFileName = file_info.canonicalFilePath();
        mFileName = file_info.fileName();
        if ( getSpatialiteLayerInfo( QString(), false, SpatialiteDbInfo::SniffMinimal ) )
        {
          // this is a Database that can be used for QgsSpatiaLiteProvider, QgsOgrProvider or QgsGdalProvider
          switch ( mDbCreateOption )
          {
            case SpatialMetadata::Spatialite40:
            case SpatialMetadata::Spatialite45:
              switch ( mSpatialMetadata )
              {
                case SpatialMetadata::SpatialiteLegacy:
                case SpatialMetadata::Spatialite40:
                case SpatialMetadata::Spatialite45:
                  // this is a Database that can be used for QgsSpatiaLiteProvider
                  bRc = true;
                  break;
                default:
                  break;
              }
              break;
            case SpatialMetadata::SpatialiteGpkg:
              switch ( mSpatialMetadata )
              {
                case SpatialMetadata::SpatialiteGpkg:
                  // this is a Database that can be used for QgsOgrProvider or QgsGdalProvider
                  bRc = true;
                  break;
                default:
                  break;
              }
              break;
            case SpatialMetadata::SpatialiteMBTiles:
              switch ( mSpatialMetadata )
              {
                case SpatialMetadata::SpatialiteMBTiles:
                  // this is a Database that can be used for QgsGdalProvider
                  bRc = true;
                  break;
                default:
                  break;
              }
              break;
            default:
              break;
          }
        }
        mDbCreateOption = SpatialMetadata::SpatialUnknown;
      }
    }
  }
  if ( mSqliteHandle )
  {
    // all done: closing the DB connection
    QgsSqliteHandle::sqlite3_close( mSqliteHandle );
    mSqliteHandle = nullptr;
  }
  //---------------------------------------------------------------
  // if true, the created Database is valid and of the Container-Type requested
  //---------------------------------------------------------------
  return bRc;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::createDatabaseSpatialite
//-----------------------------------------------------------------
bool SpatialiteDbInfo::createDatabaseSpatialite()
{
  bool bRc = false;
  sqlite3_stmt *stmt = nullptr;
  //---------------------------------------------------------------
  if ( dbHasSpatialite( true ) )
  {
    return bRc;
  }
  //---------------------------------------------------------------
  QString sql = QString( "SELECT InitSpatialMetadata(1)" );
  if ( ( mSpatialiteVersionMajor < 4 ) || ( ( mSpatialiteVersionMajor == 4 ) && ( mSpatialiteVersionMinor < 1 ) ) )
  {
    sql = QString( "SELECT InitSpatialMetadata()" );
  }
  int i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
  if ( i_rc == SQLITE_OK )
  {
    while ( sqlite3_step( stmt ) == SQLITE_ROW )
    {
      if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
      {
        if ( sqlite3_column_int( stmt, 0 ) == 1 )
        {
          bRc = true;
        }
      }
    }
    sqlite3_finalize( stmt );
  }
  //---------------------------------------------------------------
  // For Spatialite Version GE 4.2.0 only
  // - when requesed
  //---------------------------------------------------------------
  if ( ( bRc ) && ( mDbCreateOption  == SpatialMetadata::Spatialite45 ) &&
       ( ( ( mSpatialiteVersionMajor > 4 ) || ( ( mSpatialiteVersionMajor == 4 ) && ( mSpatialiteVersionMinor >= 2 ) ) ) ) )
  {
    // Will create a strictly enforced SLD/SE Styled Layers table's
    // with the corresponding Raster/VectorCoveragesTable's in a Transaction
    // - for VectorCoveragesTable: since 4.5.0
    // When needed, 'SELECT CreateTopoTables()' for Topology support should be called, but not here.
    sql = QString( "SELECT CreateStylingTables(0,1)" );
    bRc = false;
    i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
        {
          if ( sqlite3_column_int( stmt, 0 ) == 1 )
          {
            bRc = true;
          }
        }
      }
      sqlite3_finalize( stmt );
    }
  }
  //---------------------------------------------------------------
  return bRc;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::createDatabaseGeoPackage
//-----------------------------------------------------------------
bool SpatialiteDbInfo::createDatabaseGeoPackage()
{
  bool bRc = false;
  QStringList sa_sql_commands;
  QString s_sql;
  int ret;
  //---------------------------------------------------------------
  if ( dbHasSpatialite( true ) )
  {
    return bRc;
  }
  //---------------------------------------------------------------
  // For Spatialite Version GE 4.1.0 only
  //---------------------------------------------------------------
  if ( ( mSpatialiteVersionMajor > 4 ) || ( ( mSpatialiteVersionMajor == 4 ) && ( mSpatialiteVersionMinor >= 1 ) ) )
  {
    //---------------------------------------------------------------
    s_sql = QStringLiteral( "BEGIN;" );
    sa_sql_commands.append( s_sql );
    //---------------------------------------------------------------
    // Note: returns void, thus no checking for result
    // - createDatabase() will fail if these tables are not created correctly
    s_sql = QStringLiteral( "SELECT gpkgCreateBaseTables();" );
    sa_sql_commands.append( s_sql );
    //---------------------------------------------------------------
    s_sql = QStringLiteral( "COMMIT;" );
    sa_sql_commands.append( s_sql );
    //---------------------------------------------------------------
    if ( sa_sql_commands.size() > 0 )
    {
      for ( int i_list = 0; i_list < sa_sql_commands.size(); i_list++ )
      {
        s_sql = sa_sql_commands.at( i_list );
        ret = sqlite3_exec( dbSqliteHandle(), s_sql.toUtf8().constData(), NULL, NULL, nullptr );
        if ( ret != SQLITE_OK )
        {
          bRc = false;
          sa_sql_commands.clear();
          return bRc;
        }
      }
      sa_sql_commands.clear();
      bRc = true;
    }
  }
  //---------------------------------------------------------------
  return bRc;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::createDatabaseMBTiles
// - creates a View-base MbTiles file
//-----------------------------------------------------------------
bool SpatialiteDbInfo::createDatabaseMBTiles()
{
  bool bRc = false;
  QStringList sa_sql_commands;
  QString s_sql;
  int ret;
  //---------------------------------------------------------------
  s_sql = QStringLiteral( "BEGIN;" );
  sa_sql_commands.append( s_sql );
  //---------------------------------------------------------------
  s_sql = QStringLiteral( "CREATE TABLE grid_key (grid_id TEXT,key_name TEXT);" );
  sa_sql_commands.append( s_sql );
  //---------------------------------------------------------------
  s_sql = QStringLiteral( "CREATE TABLE grid_utfgrid (grid_id TEXT,grid_utfgrid BLOB);" );
  sa_sql_commands.append( s_sql );
  //---------------------------------------------------------------
  s_sql = QStringLiteral( "CREATE TABLE images (tile_data BLOB,tile_id TEXT);" );
  sa_sql_commands.append( s_sql );
  //---------------------------------------------------------------
  s_sql = QStringLiteral( "CREATE TABLE keymap (key_name TEXT,key_json TEXT);" );
  sa_sql_commands.append( s_sql );
  //---------------------------------------------------------------
  s_sql = QStringLiteral( "CREATE TABLE map (zoom_level INTEGER,tile_column INTEGER,tile_row INTEGER,tile_id TEXT,grid_id TEXT);" );
  sa_sql_commands.append( s_sql );
  //---------------------------------------------------------------
  s_sql = QStringLiteral( "CREATE TABLE metadata (name TEXT,value TEXT);" );
  sa_sql_commands.append( s_sql );
  //---------------------------------------------------------------
  // Note: View-base MBTiles are more flexible
  // - permits the referencing on non-unique tiles
  //---------------------------------------------------------------
  s_sql = QStringLiteral( "CREATE VIEW tiles AS SELECT map.zoom_level AS zoom_level,map.tile_column AS tile_column,map.tile_row AS tile_row,images.tile_data AS tile_data " );
  s_sql += QStringLiteral( "FROM map JOIN images ON images.tile_id = map.tile_id;" );
  sa_sql_commands.append( s_sql );
  //---------------------------------------------------------------
  s_sql = QStringLiteral( "CREATE VIEW grids AS SELECT map.zoom_level AS zoom_level,map.tile_column AS tile_column,map.tile_row AS tile_row,grid_utfgrid.grid_utfgrid AS grid " );
  s_sql += QStringLiteral( "FROM map JOIN grid_utfgrid ON grid_utfgrid.grid_id = map.grid_id;" );
  sa_sql_commands.append( s_sql );
  //---------------------------------------------------------------
  s_sql = QStringLiteral( "CREATE VIEW grid_data AS SELECT map.zoom_level AS zoom_level,map.tile_column AS tile_column,map." );
  s_sql += QStringLiteral( "tile_row AS tile_row,keymap.key_name AS key_name,keymap.key_json AS key_json " );
  s_sql += QStringLiteral( "FROM map JOIN grid_key ON map.grid_id = grid_key.grid_id JOIN keymap ON grid_key.key_name = keymap.key_name;" );
  sa_sql_commands.append( s_sql );
  //---------------------------------------------------------------
  s_sql = QStringLiteral( "CREATE UNIQUE INDEX grid_key_lookup ON grid_key (grid_id,key_name);" );
  sa_sql_commands.append( s_sql );
  //---------------------------------------------------------------
  s_sql = QStringLiteral( "CREATE UNIQUE INDEX grid_utfgrid_lookup ON grid_utfgrid (grid_id);" );
  sa_sql_commands.append( s_sql );
  //---------------------------------------------------------------
  s_sql = QStringLiteral( "CREATE UNIQUE INDEX images_id ON images (tile_id);" );
  sa_sql_commands.append( s_sql );
  //---------------------------------------------------------------
  s_sql = QStringLiteral( "CREATE UNIQUE INDEX keymap_lookup ON keymap (key_name);" );
  sa_sql_commands.append( s_sql );
  //---------------------------------------------------------------
  s_sql = QStringLiteral( "CREATE UNIQUE INDEX map_index ON map (zoom_level,tile_column,tile_row);" );
  sa_sql_commands.append( s_sql );
  //---------------------------------------------------------------
  s_sql = QStringLiteral( "CREATE UNIQUE INDEX name ON metadata (name);" );
  sa_sql_commands.append( s_sql );
  //---------------------------------------------------------------
  s_sql = QStringLiteral( "INSERT INTO 'metadata' VALUES('name','empty.tbtiles');" );
  sa_sql_commands.append( s_sql );
  //---------------------------------------------------------------
  // baselayer or overlay
  s_sql = QStringLiteral( "INSERT INTO 'metadata' VALUES('type','baselayer');" );
  sa_sql_commands.append( s_sql );
  //---------------------------------------------------------------
  // 2013-02
  s_sql = QStringLiteral( "INSERT INTO 'metadata' VALUES('version','1.2.0');" );
  sa_sql_commands.append( s_sql );
  //---------------------------------------------------------------
  s_sql = QStringLiteral( "INSERT INTO 'metadata' VALUES('description','Minimal Requirement for MBTiles Specification 1.2');" );
  sa_sql_commands.append( s_sql );
  //---------------------------------------------------------------
  // png or jpg
  s_sql = QStringLiteral( "INSERT INTO 'metadata' VALUES('format','png');" );
  sa_sql_commands.append( s_sql );
  //---------------------------------------------------------------
  // OpenLayers Bounds format - left, bottom, right, top.
  s_sql = QStringLiteral( "INSERT INTO 'metadata' VALUES('bounds','-180,-85.0511,180,85.0511');" );
  sa_sql_commands.append( s_sql );
  //---------------------------------------------------------------
  s_sql = QStringLiteral( "INSERT INTO 'metadata' VALUES('attribution','MBTiles (1.2) is a specification for storing tiled map data in SQLite databases for immediate usage and for transfer. MBTiles files, known as tilesets, must implement the specification below to ensure compatibility with devices.');" );
  sa_sql_commands.append( s_sql );
  //---------------------------------------------------------------
  s_sql = QStringLiteral( "INSERT INTO 'metadata' VALUES('minzoom','0');" );
  sa_sql_commands.append( s_sql );
  //---------------------------------------------------------------
  s_sql = QStringLiteral( "INSERT INTO 'metadata' VALUES('maxzoom','9');" );
  sa_sql_commands.append( s_sql );
  //---------------------------------------------------------------
  s_sql = QStringLiteral( "INSERT INTO 'metadata' VALUES('center','13.3777,52.5162,9');" );
  sa_sql_commands.append( s_sql );
  //---------------------------------------------------------------
  s_sql = QStringLiteral( "INSERT INTO 'metadata' VALUES('template','');" );
  sa_sql_commands.append( s_sql );
  //---------------------------------------------------------------
  // Optional fields for Copyright and License
  //---------------------------------------------------------------
  s_sql = QStringLiteral( "INSERT INTO 'metadata' VALUES('copyright','');" );
  sa_sql_commands.append( s_sql );
  // INSERT INTO metadata (name,value) SELECT "copyright","Tolkien Estate, Permission for use requested: 2017-07-31 11:31 CEST";
  //---------------------------------------------------------------
  s_sql = QStringLiteral( "INSERT INTO 'metadata' VALUES('license','');" );
  sa_sql_commands.append( s_sql );
  // INSERT INTO metadata (name,value) SELECT "license","Proprietary - Non Free";
  //---------------------------------------------------------------
  // tms or oms - not official
  s_sql = QStringLiteral( "INSERT INTO 'metadata' VALUES('tile_row_type','tms');" );
  sa_sql_commands.append( s_sql );
  //---------------------------------------------------------------
  s_sql = QStringLiteral( "COMMIT;" );
  sa_sql_commands.append( s_sql );
  //---------------------------------------------------------------
  if ( sa_sql_commands.size() > 0 )
  {
    for ( int i_list = 0; i_list < sa_sql_commands.size(); i_list++ )
    {
      s_sql = sa_sql_commands.at( i_list );
      ret = sqlite3_exec( dbSqliteHandle(), s_sql.toUtf8().constData(), NULL, NULL, nullptr );
      if ( ret != SQLITE_OK )
      {
        bRc = false;
        sa_sql_commands.clear();
        return bRc;
      }
    }
    sa_sql_commands.clear();
    bRc = true;
  }
  //---------------------------------------------------------------
  return bRc;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::checkMBTiles
//-----------------------------------------------------------------
bool SpatialiteDbInfo::checkMBTiles()
{
  bool bRc = false;
  //---------------------------------------------------------------
  // Note: no checking for IsValid while we are 'sniffing' the Database
  //---------------------------------------------------------------
  sqlite3_stmt *stmt = nullptr;
  //--------------------------------------------------------
  // MBTiles: at least 2 specific tables ['metadata' alone can often be found elsewhere]
  QString sql = QStringLiteral( "SELECT count(tbl_name) FROM sqlite_master WHERE ((type = 'table' OR type = 'view') AND (tbl_name IN ('metadata','tiles','images','map')))" );
  int i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );

  if ( i_rc == SQLITE_OK )
  {
    while ( sqlite3_step( stmt ) == SQLITE_ROW )
    {
      if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
      {
        mHasMBTilesTables = sqlite3_column_int( stmt, 0 );
      }
    }
    sqlite3_finalize( stmt );
  }
  if ( mHasMBTilesTables > 0 )
  {
    bRc = true;
  }
  //---------------------------------------------------------------
  return bRc;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::readMBTilesLayers
//-----------------------------------------------------------------
bool SpatialiteDbInfo::readMBTilesLayers()
{
  bool bRc = false;
  //---------------------------------------------------------------
  if ( isDbValid() )
  {
    sqlite3_stmt *stmt = nullptr;
    SpatialiteLayerType type_MBTiles = MBTilesTable;
    if ( mHasMBTilesTables > 2 )
    {
      type_MBTiles = MBTilesView;
    }
    mMBTilesLayers.clear();
    mHasMBTilesTables = 0; // Assume invalid ; 1 is valid (there can only be 1 Raster-Layer)
    //--------------------------------------------------------
    // The 'metadata' table must contain in the column 'name'
    // version: 1.0.0:
    // - name, description, version
    // version: 1.1:
    // - name, description, version,format
    // The 'tiles' table or view must not be empty
    //--------------------------------------------------------
    QString sql = QStringLiteral( "SELECT  " );
    sql += QStringLiteral( "(SELECT count(name) FROM 'metadata' " );
    sql += QStringLiteral( "WHERE (name IN ('name','description','version','format')))," );
    sql += QStringLiteral( "(SELECT count(zoom_level) FROM tiles LIMIT 1)," );
    sql += QStringLiteral( "(SELECT value FROM 'metadata' WHERE (name IN ('name')))" );
    // SELECT (SELECT count(name) FROM 'metadata' WHERE (name IN ('name','description','version','format'))) AS valid, (SELECT count(zoom_level) FROM tiles LIMIT 1) AS count, (SELECT value FROM 'metadata' WHERE (name IN ('name'))) AS name
    int i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      QString sDataSourceUri;
      QString sLayerInfo;
      int iSrid = 4326;
      int iSpatialIndex = GAIA_VECTOR_UNKNOWN;
      int iIsHidden = 0;
      QString sLayerType = SpatialiteDbInfo::SpatialiteLayerTypeName( type_MBTiles );
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL ) )
        {
          if ( ( sqlite3_column_int( stmt, 0 ) >= 3 ) && ( sqlite3_column_int( stmt, 1 ) > 0 ) )
          {
            // Use file-name (excluding the path) as LayerName
            // - metadata name/value may not be unique across different mbtiles-files [should be used only as title]
            QString sNameValue = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 2 ) );
            createDbLayerInfoUri( sLayerInfo, sDataSourceUri, sNameValue, type_MBTiles, sLayerType, iSrid, iSpatialIndex, iIsHidden );
            mMBTilesLayers.insert( getFileName(), sLayerInfo );
          }
        }
      }
      sqlite3_finalize( stmt );
    }
    //--------------------------------------------------------
    mHasMBTilesTables = mMBTilesLayers.count();
    if ( mHasMBTilesTables > 0 )
    {
      bRc = true;
      sql = QStringLiteral( "MBTiles" );
      GDALDriverH mbtilesGdalDriver = GDALGetDriverByName( sql.toLocal8Bit().constData() );
      if ( mbtilesGdalDriver )
      {
        mHasGdalMBTilesDriver = true;
        mIsGdalOgr = true;
      }
    }
  }
  //---------------------------------------------------------------
  return bRc;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::readGeoPackageLayers
//-----------------------------------------------------------------
bool SpatialiteDbInfo::readGeoPackageLayers()
{
  bool bRc = false;
  //---------------------------------------------------------------
  if ( isDbValid() )
  {
    mHasGeoPackageTables = 0;
    mHasGeoPackageVectors = 0;
    mHasGeoPackageRasters = 0;
    mGeoPackageLayers.clear();
    sqlite3_stmt *stmt = nullptr;
    sqlite3_stmt *stmtSubquery = nullptr;
    //--------------------------------------------------------
    //  SELECT table_name, data_type, identifier, srs_id FROM gpkg_contents WHERE (data_type IN ('features','tiles'))
    //--------------------------------------------------------
    // We are checking if valid, thus the (otherwise not needed) extra WHERE
    // PRAGMA table_info(gpkg_contents);
    QString sql = QStringLiteral( "SELECT table_name, data_type, srs_id FROM gpkg_contents WHERE (data_type IN ('features','tiles'))" );
    int i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        QString sDataSourceUri;
        QString sLayerInfo;
        int iSpatialIndex = GAIA_VECTOR_UNKNOWN;
        int iIsHidden = 0;
        if ( ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL ) )
        {
          QString sTableName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
          QString sLayerName = sTableName;
          QString sLayerType = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
          int iSrid = sqlite3_column_int( stmt, 2 ) ;
          SpatialiteLayerType type_GeoPackage = GeoPackageVector;
          QString sProvider = "ogr";
          if ( sLayerType == "tiles" )
          {
            sProvider = "gdal";
            type_GeoPackage = GeoPackageRaster;
            sLayerType = SpatialiteDbInfo::SpatialiteLayerTypeName( type_GeoPackage );
            createDbLayerInfoUri( sLayerInfo, sDataSourceUri, sLayerName, type_GeoPackage, sLayerType, iSrid, iSpatialIndex, iIsHidden );
            mHasGeoPackageRasters++;
          }
          if ( type_GeoPackage == GeoPackageVector )
          {
            QString sFields = QString( "column_name, geometry_type_name,z,m" );
            sql = QStringLiteral( "SELECT %1 FROM gpkg_geometry_columns WHERE (table_name='%2')" ).arg( sFields ).arg( sTableName );
            i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmtSubquery, nullptr );
            if ( i_rc == SQLITE_OK )
            {
              while ( sqlite3_step( stmtSubquery ) == SQLITE_ROW )
              {
                if ( ( sqlite3_column_type( stmtSubquery, 0 ) != SQLITE_NULL ) &&
                     ( sqlite3_column_type( stmtSubquery, 1 ) != SQLITE_NULL ) &&
                     ( sqlite3_column_type( stmtSubquery, 2 ) != SQLITE_NULL ) &&
                     ( sqlite3_column_type( stmtSubquery, 3 ) != SQLITE_NULL ) )
                {
                  QString sGeometryColumn = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmtSubquery, 0 ) );
                  QString sGeometryType = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmtSubquery, 1 ) );
                  QString sGeometryDimension = "XY";
                  int isZ = sqlite3_column_int( stmtSubquery, 2 ) ;
                  if ( isZ == 1 )
                  {
                    sGeometryDimension = QString( "%1%2" ).arg( sGeometryDimension ).arg( "Z" );
                  }
                  int isM = sqlite3_column_int( stmtSubquery, 3 );
                  if ( isM == 1 )
                  {
                    sGeometryDimension = QString( "%1%2" ).arg( sGeometryDimension ).arg( "M" );
                  }
                  if ( !sGeometryColumn.isEmpty() )
                  {
                    sLayerName = QString( "%1(%2)" ).arg( sTableName ).arg( sGeometryColumn );
                  }
                  sGeometryType = QgsWkbTypes::displayString( SpatialiteDbLayer::GetGeometryTypeLegacy( sGeometryType, sGeometryDimension ) );
                  createDbLayerInfoUri( sLayerInfo, sDataSourceUri, sLayerName, type_GeoPackage, sGeometryType, iSrid, iSpatialIndex, iIsHidden );
                  mHasGeoPackageVectors++;
                }
              }
              sqlite3_finalize( stmtSubquery );
            }
          }
          mGeoPackageLayers.insert( sLayerName, sLayerInfo );
        }
      }
      sqlite3_finalize( stmt );
    }
    mHasGeoPackageTables = mGeoPackageLayers.count();
    if ( mHasGeoPackageTables > 0 )
    {
      bRc = true;
      sql = QStringLiteral( "GPKG" );
      GDALDriverH gpkgGdalDriver = GDALGetDriverByName( sql.toLocal8Bit().constData() );
      if ( gpkgGdalDriver )
      {
        mHasGdalGeoPackageDriver = true;
        mIsGdalOgr = true;
      }
    }
  }
  return bRc;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::readFdoOgrLayers
//-----------------------------------------------------------------
bool SpatialiteDbInfo::readFdoOgrLayers()
{
  bool bRc = false;
  //---------------------------------------------------------------
  if ( isDbValid() )
  {
    mHasFdoOgrTables = 0;
    mFdoOgrLayers.clear();
    sqlite3_stmt *stmt = nullptr;
    //---------------------------------------------------------------
    // Note: Both Spatialite and GdalFdoOgr have a 'geometry_columns' TABLE
    // - The GdalFdoOgr version contains a column 'geometry_format', The Spatialite version does not.
    //---------------------------------------------------------------
    QString sql = QStringLiteral( "SELECT  f_table_name, f_geometry_column, srid,geometry_type,coord_dimension FROM 'geometry_columns'" );
    int i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
    //--------------------------------------------------------
    if ( i_rc == SQLITE_OK )
    {
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        QString sDataSourceUri;
        QString sLayerInfo;
        int iSpatialIndex = GAIA_VECTOR_UNKNOWN;
        int iIsHidden = 0;
        if ( ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 3 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 4 ) != SQLITE_NULL ) )
        {
          QString sTableName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
          QString sGeometryName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
          QString sLayerName = QString( "%1(%2)" ).arg( sTableName ).arg( sGeometryName );
          int iSrid = sqlite3_column_int( stmt, 2 );
          QString sGeometryType = QgsWkbTypes::displayString( SpatialiteDbLayer::GetGeometryType( sqlite3_column_int( stmt, 3 ), sqlite3_column_int( stmt, 4 ) ) );
          createDbLayerInfoUri( sLayerInfo, sDataSourceUri, sLayerName, SpatialiteDbInfo::GdalFdoOgr, sGeometryType, iSrid, iSpatialIndex, iIsHidden );
          mFdoOgrLayers.insert( sLayerName, sLayerInfo );
        }
      }
      sqlite3_finalize( stmt );
    }
    //--------------------------------------------------------
    mHasFdoOgrTables = mFdoOgrLayers.count();
    if ( mHasFdoOgrTables > 0 )
    {
      bRc = true;
      sql = QStringLiteral( "SQLite" );
      GDALDriverH fdoGdalDriver = GDALGetDriverByName( sql.toLocal8Bit().constData() );
      if ( fdoGdalDriver )
      {
        mHasFdoOgrDriver = true;
        mIsGdalOgr = true;
      }
    }
  }
  //---------------------------------------------------------------
  return bRc;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::dropGeoTable
//-----------------------------------------------------------------
// For Spatialite between 4.0 and 4.1: the Api-Command 'gaiaDropTable' will be used
// For Spatialite versions >=  4.2 : the Sql-Command 'DropGeoTable' will be used
// For Spatialite version < 4.0 or non Spatialite TABLES: an error will be returned
//-----------------------------------------------------------------
bool SpatialiteDbInfo::dropGeoTable( QString sTableName, QString &errCause )
{
  bool bRc = false;
  //---------------------------------------------------------------
  // Spatialite specific
  //---------------------------------------------------------------
  if ( ( isDbValid() ) && ( isDbSpatialite() ) )
  {
    QString sql = QString();
    char *errMsg = nullptr;
    int i_rc = SQLITE_OK; // For Spatialite this is an error [not true]
    // Force a connection if needed (with Spatialite Version number being used)
    if ( dbHasSpatialite( true ) )
    {
      if ( dbSpatialiteVersionMajor() >= 4 )
      {
        // Both the Sql and Api-Versions remove the Spatial-Index and all corresponding entries
        if ( ( ( dbSpatialiteVersionMajor() == 4 ) && ( dbSpatialiteVersionMinor() > 2 ) ) ||
             ( dbSpatialiteVersionMajor() > 4 ) )
        {
          sqlite3_stmt *stmt = nullptr;
          // Use the Sql-DropGeoTable command
          sql = QStringLiteral( "SELECT DropGeoTable('%1')" ).arg( sTableName );
          i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
          QString errorMessage = QString( "%1" ).arg( sqlite3_errmsg( dbSqliteHandle() ) );
          //--------------------------------------------------------
          if ( i_rc == SQLITE_OK )
          {
            while ( sqlite3_step( stmt ) == SQLITE_ROW )
            {
              if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
              {
                // We must query the Spatialite return code [1=correct ; 0=error]
                i_rc = sqlite3_column_int( stmt, 0 );
              }
            }
            sqlite3_finalize( stmt );
            if ( i_rc == SQLITE_OK )
            {
              // For Spatialite this is an error [not true]
              errCause = QString( "[SpatialiteDbInfo::dropGeoTable] failed rc=%1 sql[%2]" ).arg( i_rc ).arg( sql );
              return bRc;
            }
          }
          else
          {
            // Possibly a prepair error [invalid-table]
            errCause = QString( "[SpatialiteDbInfo::dropGeoTable] failed error[%1] sql[%2]" ).arg( errorMessage ).arg( sql );
            return bRc;
          }
        }
        else
        {
          if ( !gaiaDropTable( dbSqliteHandle(), sTableName.toUtf8().constData() ) )
          {
            // unexpected error
            errCause = QObject::tr( "[SpatialiteDbInfo::dropGeoTable] Unable to delete table [%1]\n" ).arg( sTableName );
            return bRc;
          }
        }
        // All Layers contained in this TABLE will be removed, since they no longer exist
        removeGeoTable( sTableName );
        sql = QStringLiteral( "VACUUM" );
        i_rc = sqlite3_exec( dbSqliteHandle(), sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
        if ( i_rc != SQLITE_OK )
        {
          errCause = QString( "[SpatialiteDbInfo::dropGeoTable] Failed to run VACUUM rc=%1, after deleting table[%2] on database [%3] error[%4]" ).arg( i_rc ).arg( sTableName ).arg( getDatabaseFileName() ).arg( errMsg );
          return bRc;
        }
        bRc = true; // This is not an error.
        errCause = QString( "[SpatialiteDbInfo::dropGeoTable] GeoTable[%1] was dropped and a VACUUM performed." ).arg( sTableName );
      }
      else
      {
        errCause = QString( "[SpatialiteDbInfo::dropGeoTable] This function is not supported with the running version of Spatialite [%1]" ).arg( dbSpatialiteVersionInfo() );
      }
    }
    else
    {
      errCause = QString( "[SpatialiteDbInfo::dropGeoTable] This function needs Spatialite. Could not connect. [%1]" ).arg( dbSpatialiteVersionInfo() );
    }
  }
  else
  {
    errCause = QObject::tr( "[SpatialiteDbInfo::dropGeoTable] Unable to delete table [%1]: not a Spatialite Database.\n" ).arg( sTableName ).arg( getFileMimeTypeString() );
  }
  //---------------------------------------------------------------
  return bRc;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::checkLayerSanity
//-----------------------------------------------------------------
int SpatialiteDbInfo::checkLayerSanity( QString sLayerName )
{
  int i_rc = 101;
  char *errMsg = nullptr;
  QString sTableName = QString::null;
  // if 'sGeometryColumn' remains empty, the row count will be returned
  QString sGeometryColumn = QString::null;
  //----------------------------------------------------------
  // Extract TableName/GeometryColumn from sent 'table_name' or 'table_name(field_name)' from LayerName
  if ( SpatialiteDbInfo::parseLayerName( sLayerName, sTableName, sGeometryColumn ) )
  {
    //---------------------------------------------------------------
    QString sql = QStringLiteral( "SELECT count('%2') FROM '%1' LIMIT 1" ).arg( sTableName ).arg( sGeometryColumn );
    i_rc = sqlite3_exec( dbSqliteHandle(), sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
    if ( i_rc != SQLITE_OK )
    {
      sql = QString( "%1 rc=%2" ).arg( errMsg ).arg( i_rc );
      if ( sql.contains( "no such table" ) )
      {
        i_rc = 100;
      }
      else if ( sql.contains( "no such column" ) )
      {
        i_rc = 101;
      }
      else if ( sql.contains( "is not a function" ) )
      {
        // 'SpatialView': possible faulty Sql-Syntax
        i_rc = 200;
      }
    }
  }
  //---------------------------------------------------------------
  return i_rc;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::GetDbLayersInfo
//-----------------------------------------------------------------
// To avoid a possible loop, 2 results are returned
// - If the LayerName was found in one of the Map-Lists
// -> 'bFoundLayerName' will return true
// When the LayerName is not empty and 'bFoundLayerName' is true
// - but bRc is false: the Layer contains errors and cannot be created
// - when called from a project: QgsHandleBadLayers will be called
//-----------------------------------------------------------------
bool SpatialiteDbInfo::GetDbLayersInfo( QString sLayerName, bool &bFoundLayerName )
{
  bool bRc = false;
  bFoundLayerName = false;
  if ( isDbValid() )
  {
    //---------------------------------------------------------------
    if ( !sLayerName.isEmpty() )
    {
      // Note: for some reason this is being called with the uri ; extract the LayerName in such cases
      if ( ( sLayerName.contains( "dbname=" ) ) && ( sLayerName.contains( "table=" ) ) )
      {
        QgsDataSourceUri anUri = QgsDataSourceUri( sLayerName );
        sLayerName = anUri.table();
        if ( !anUri.geometryColumn().toLower().isEmpty() )
        {
          sLayerName = QString( "%1(%2)" ).arg( sLayerName ).arg( anUri.geometryColumn().toLower() );
        }
      }
    }
    if ( mHasVectorLayers > 0 )
    {
      if ( ( sLayerName.isEmpty() ) || ( mVectorLayers.contains( sLayerName ) ) )
      {
        // All Layers will be loaded if sLayerName is empty
        // - otherwise load only a specific Layer
        // -> will load Spatialite connection , if not done allready
        bRc = GetVectorLayersInfo( sLayerName );
        if ( mVectorLayers.contains( sLayerName ) )
        {
          // LayerName was found ; bRc returns if a Layer was created
          bFoundLayerName = true;
        }
      }
    }
    if ( !bFoundLayerName )
    {
      if ( mHasRasterCoveragesTables > 0 )
      {
        if ( ( sLayerName.isEmpty() ) || ( mRasterCoveragesLayers.contains( sLayerName ) ) )
        {
          // All Layers will be loaded if sLayerName is empty
          // - otherwise load only a specific Layer
          // -> will load Spatialite, with RasterLite2, connection , if not done allready
          bRc = GetRasterLite2RasterLayersInfo( sLayerName );
          if ( mRasterCoveragesLayers.contains( sLayerName ) )
          {
            // LayerName was found ; bRc returns if a Layer was created
            bFoundLayerName = true;
          }
        }
      }
      if ( mHasTopologyExportTables > 0 )
      {
        if ( ( sLayerName.isEmpty() ) || ( mTopologyExportLayers.contains( sLayerName ) ) )
        {
          // All Layers will be loaded if sLayerName is empty
          // - otherwise load only a specific Layer
          // -> will load Spatialite connection , if not done allready
          bRc = GetTopologyLayersInfo( sLayerName );
          if ( !bFoundLayerName )
          {
            if ( mTopologyExportLayers.contains( sLayerName ) )
            {
              // LayerName was found ; bRc returns if a Layer was created
              bFoundLayerName = true;
            }
          }
        }
      }
      if ( mHasRasterLite1Tables > 0 )
      {
        if ( ( sLayerName.isEmpty() ) || ( mRasterLite1Layers.contains( sLayerName ) ) )
        {
          // All Layers will be loaded if sLayerName is empty
          // - otherwise load only a specific Layer
          bRc = GetRasterLite1LayersInfo( sLayerName );
          if ( mRasterLite1Layers.contains( sLayerName ) )
          {
            // LayerName was found ; bRc returns if a Layer was created
            bFoundLayerName = true;
          }
        }
      }
      if ( mHasMBTilesTables > 0 )
      {
        if ( ( sLayerName.isEmpty() ) || ( mMBTilesLayers.contains( sLayerName ) ) )
        {
          // All Layers will be loaded if sLayerName is empty
          // - otherwise load only a specific Layer
          bRc = GetMBTilesLayersInfo( sLayerName );
          if ( mMBTilesLayers.contains( sLayerName ) )
          {
            // LayerName was found ; bRc returns if a Layer was created
            bFoundLayerName = true;
          }
        }
      }
      if ( mHasGeoPackageTables > 0 )
      {
        if ( ( sLayerName.isEmpty() ) || ( mGeoPackageLayers.contains( sLayerName ) ) )
        {
          // All Layers will be loaded if sLayerName is empty
          // - otherwise load only a specific Layer
          bRc = GetGeoPackageLayersInfo( sLayerName );
          if ( mGeoPackageLayers.contains( sLayerName ) )
          {
            // LayerName was found ; bRc returns if a Layer was created
            bFoundLayerName = true;
          }
        }
      }
      if ( mHasFdoOgrTables > 0 )
      {
        if ( ( sLayerName.isEmpty() ) || ( mFdoOgrLayers.contains( sLayerName ) ) )
        {
          // All Layers will be loaded if sLayerName is empty
          // - otherwise load only a specific Layer
          bRc = GetFdoOgrLayersInfo( sLayerName );
          if ( mFdoOgrLayers.contains( sLayerName ) )
          {
            // LayerName was found ; bRc returns if a Layer was created
            bFoundLayerName = true;
          }
        }
      }
      // Must run after the other Layers have been created
      if ( mHasVectorCoveragesTables > 0 )
      {
        // TODO: remove this
        // bRc = GetRasterLite2VectorLayersInfo( sLayerName );
        // QgsDebugMsgLevel( QString( "-I-> SpatialiteDbInfo::GetDbLayersInfo(%1) : [%3] bRc[%2]" ).arg( sLayerName ).arg( bRc ).arg( "GetRasterLite2VectorLayersInfo" ), 4);
        if ( !sLayerName.isEmpty() )
        {
          // LayerName was found ; bRc returns if a Layer was created
          // bFoundLayerName = bRc;
        }
      }
      if ( !bFoundLayerName )
      {
        mNonSpatialTables[ sLayerName ] = "Data-Table";
      }
    }
    prepare(); // Ignore bChanged return code [if a Layer was created must be returned]
  }
  //-----------------------------------------------------------------
  // if  bRc is true, a Layer was created: this will not be called again
  // if LayerName was found but bRc is false: this will not be called again [avoids a loop]
  // - when called from a project: QgsHandleBadLayers will be called
  // if both are false an alternitve LayerName may call this again
  //-----------------------------------------------------------------
  return bRc;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::GetVectorLayersInfo
//-----------------------------------------------------------------
// All Layers will be loaded if sLayerName is empty
// - otherwise load only a specific Layer
// -> will load Spatialite connection , if not done allready
//-----------------------------------------------------------------
bool SpatialiteDbInfo::GetVectorLayersInfo( const QString sLayerName )
{
  bool bRc = false;
  if ( isDbValid() )
  {
    int i_check_count_vectors = mHasVectorLayers;
    QStringList saSearchTables;
    bool bFoundLayerName = false;
    if ( !sLayerName.isEmpty() )
    {
      // Assume a layer is being loaded
      // - will load Spatialite connection , if not done allready
      if ( !dbHasSpatialite( true ) )
      {
        return bRc;
      }
      if ( mVectorLayers.contains( sLayerName ) )
      {
        // Searching only for a specific Layer
        saSearchTables.append( sLayerName );
      }
    }
    else
    {
      saSearchTables = mVectorLayers.uniqueKeys();
      // Checking only when loading all Layers
      mHasVectorLayers = 0;
    }
    if ( saSearchTables.size() )
    {
      for ( int i = 0; i < saSearchTables.size(); i++ )
      {
        QString sSearchLayer = saSearchTables.at( i );
        if ( mVectorLayers.contains( sSearchLayer ) )
        {
          // Add only if it does not already exist [using true here, would cause loop]
          if ( !getSpatialiteDbLayer( sSearchLayer, false ) )
          {
            QString sTableName  = sSearchLayer;
            QString sGeometryColumn = QString::null;
            QString sLayerType = QString::null;
            int iLayerSrid = -1;
            QString sGeometryType = QString::null; // For Layers that contain no Geometries, this will be the Layer-Type
            QString sProvider = QString::null;
            // SpatialIndex, AuthInfos->IsHidden
            int iSpatialIndex = GAIA_VECTOR_UNKNOWN;
            int iIsHidden = 0;
            bool bIsHidden = false; // TODO
            QString sLayerValue = QString::null;
            //----------------------------------------------------------
            // Extract TableName/GeometryColumn from sent 'table_name' or 'table_name(field_name)' from LayerName
            SpatialiteDbInfo::parseLayerName( sSearchLayer, sTableName, sGeometryColumn );
            //----------------------------------------------------------
            sLayerValue = mVectorLayers.value( sSearchLayer );
            if ( parseLayerInfo( sLayerValue, sGeometryType, iLayerSrid, sProvider, sLayerType, iSpatialIndex, iIsHidden ) )
            {
              SpatialiteDbLayer *dbLayer = new SpatialiteDbLayer( this );
              if ( dbLayer )
              {
                dbLayer->mLayerId = mLayersCount++;
                bIsHidden = ( bool )iIsHidden;
                dbLayer->setLayerType( SpatialiteDbInfo::SpatialiteLayerTypeFromName( sLayerType ) );
                dbLayer->mIsLayerValid = true;
                dbLayer->mTableName = sTableName;
                dbLayer->mLayerName = dbLayer->mTableName;
                dbLayer->mGeometryColumn = sGeometryColumn;
                dbLayer->setGeometryType( QgsWkbTypes::parseType( sGeometryType ) );
                if ( dbLayer->mGeometryType == QgsWkbTypes::Unknown )
                {
                  dbLayer->setLayerInvalid( dbLayer->mLayerName, QString( "GetVectorLayersInfo [%1] Capabilities: Invalid GeometryrType[%2]." ).arg( sSearchLayer ).arg( dbLayer->getGeometryTypeString() ) );
                }
                if ( !dbLayer->mGeometryColumn.isEmpty() )
                {
                  dbLayer->mLayerName = QString( "%1(%2)" ).arg( dbLayer->mTableName ).arg( dbLayer->mGeometryColumn );
                }
                dbLayer->setSrid( iLayerSrid );
                if ( dbLayer->mTableName.isEmpty() )
                {
                  dbLayer->setLayerInvalid( dbLayer->mLayerName, QString( "[%1] Invalid Table-Name[%2]: no result" ).arg( sSearchLayer ).arg( dbLayer->mTableName ) );
                }
                if ( dbLayer->mGeometryColumn.isEmpty() )
                {
                  dbLayer->setLayerInvalid( dbLayer->mLayerName, QString( "[%1] Invalid GeometryColumn-Name[%2]: no result" ).arg( sSearchLayer ).arg( dbLayer->mGeometryColumn ) );
                }
                if ( ( dbLayer->mTableName.isEmpty() ) || ( dbLayer->mGeometryColumn.isEmpty() ) || ( dbLayer->mSrid < -1 ) )
                {
                  dbLayer->setLayerInvalid( dbLayer->mLayerName, QString( "[%1] Invalid Srid[%2]: no result" ).arg( sSearchLayer ).arg( dbLayer->mSrid ) );
                }
                dbLayer->setSpatialIndexType( iSpatialIndex );
                if ( dbLayer->mSpatialIndexType == SpatialiteDbInfo::SpatialIndexNone )
                {
                  dbLayer->setLayerInvalid( dbLayer->mLayerName, QString( "GetVectorLayersInfo [%1] Invalid SpatialiIndex[%2]." ).arg( sSearchLayer ).arg( dbLayer->getSpatialIndexString() ) );
                }
                dbLayer->mLayerIsHidden = bIsHidden;
                if ( dbLayer->mLayerIsHidden )
                {
                  dbLayer->setLayerInvalid( dbLayer->mLayerName, QString( "GetVectorLayersInfo [%1] LayerIsHidden" ).arg( sSearchLayer ) );
                }
                if ( dbLayer->isLayerValid() )
                {
                  // Will read and set LayerExtent and NumberFeatures
                  dbLayer->getLayerExtent( true );
                  if ( dbLayer->mLayerType == SpatialiteDbInfo::SpatialTable )
                  {
                    dbLayer->mLayerReadOnly = 0; // default is 1
                  }
                  dbLayer->mLayerIsHidden = bIsHidden;
                  if ( dbLayer->mLayerType == SpatialiteDbInfo::SpatialView )
                  {
                    dbLayer->GetViewTriggers();
                  }
                }
                //-----------------------------------------
                //---retrieving Table settings ----------
                //---retrieving common settings --------
                //---retrieving View settings -----------
                //---setting EnabledCapabilities---------
                //-----------------------------------------
                if ( dbLayer->isLayerValid() )
                {
                  if ( dbLayer->GetLayerSettings() )
                  {
                    //-----------------------------------------
                    // Layer isValid
                    switch ( dbLayer->mLayerType )
                    {
                      case SpatialiteDbInfo::SpatialTable:
                        if ( dbLayer->mSpatialIndexType == SpatialiteDbInfo::SpatialIndexRTree )
                        {
                          // Replace type
                          QString sIndexName = QString( "idx_%1_%2" ).arg( dbLayer->mTableName ).arg( dbLayer->mGeometryColumn );
                          mNonSpatialTables[ sIndexName ] = "SpatialIndex-Table";
                          mNonSpatialTables[QString( "%1_node" ).arg( sIndexName ) ] = "SpatialIndex-Node";
                          mNonSpatialTables[QString( "%1_parent" ).arg( sIndexName ) ] = "SpatialIndex-Parent";
                          mNonSpatialTables[QString( "%1_rowid" ).arg( sIndexName ) ] = "SpatialIndex-Rowid";
                        }
                        break;
                      case SpatialiteDbInfo::SpatialView:
                        break;
                      case SpatialiteDbInfo::VirtualShape:
                        break;
                      default:
                        break;
                    }
                    if ( dbLayer->mLayerName == sLayerName )
                    {
                      // We have found what we are looking for, prevent RL1/2 and Topology search
                      bFoundLayerName = true;
                    }
                    // Remove valid Layer Table-names
                    mNonSpatialTables.remove( dbLayer->mTableName );
                    mDbLayers.insert( dbLayer->mLayerName, dbLayer );
                    if ( sLayerName.isEmpty() )
                    {
                      mHasVectorLayers++;
                    }
                  }
                }
                if ( !dbLayer->isLayerValid() )
                {
                  mErrors.insert( dbLayer->mLayerName, QString( "-W-> SpatialiteDbInfo::GetVectorLayersInfo Errors[%1] LayerType[%2] Layername[%3]" ).arg( ( dbLayer->getErrors().size() + 1 ) ).arg( dbLayer->mLayerTypeString ).arg( dbLayer->mLayerName ) );
                  delete dbLayer;
                  dbLayer = nullptr;
                }
              }
            }
            else
            {
              int i_reason = checkLayerSanity( sSearchLayer );
              switch ( i_reason )
              {
                case 100:
                  mErrors.insert( sSearchLayer, QString( "GetVectorLayersInfo: cause: Table not found[%1]" ).arg( sSearchLayer ) );
                  break;
                case 101:
                  mErrors.insert( sSearchLayer, QString( "GetVectorLayersInfo: cause: Column not found[%1]" ).arg( sSearchLayer ) );
                  break;
                case 200:
                  mErrors.insert( sSearchLayer, QString( "GetVectorLayersInfo: possible cause: Sql-Syntax error when Layer is a SpatialView[%1]" ).arg( sSearchLayer ) );
                  break;
                default:
                  mErrors.insert( sSearchLayer, QString( "GetVectorLayersInfo: Unknown error[%1]" ).arg( sSearchLayer ) );
                  break;
              }
            }
          }
          else
          {
            // We have found what we are looking for, prevent RL1/2 and Topology search
            bFoundLayerName = true;
            // QgsDebugMsgLevel( QString( "-W->  SpatialiteDbInfo::GetDbLayersInfo[%2] SearchLayer[%1]" ).arg( sSearchLayer ).arg( i ), 4 );
          }
        }
      }
    }
    if ( i_check_count_vectors != mHasVectorLayers )
    {
      // TODO possible warning that something was invalid
    }
    if ( !sLayerName.isEmpty() )
    {
      if ( bFoundLayerName )
      {
        bRc = bFoundLayerName;
      }
    }
    else
    {
      bRc = isDbValid();
    }
  }
  return bRc;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::GetRasterLite2RasterLayersInfo
//-----------------------------------------------------------------
// All Layers will be loaded if sLayerName is empty
// - otherwise load only a specific Layer
// -> will load Spatialite, with RasterLite2, connection , if not done allready
//-----------------------------------------------------------------
bool SpatialiteDbInfo::GetRasterLite2RasterLayersInfo( const QString sLayerName )
{
  bool bRc = false;
  if ( isDbValid() )
  {
    sqlite3_stmt *stmt = nullptr;
    int i_rc = 0;
    bool bFoundLayerName = false;
    QString sTableName  = QString::null;
    QString sGeometryColumn = QString::null;
    QString sql = QString::null;
    int i_check_count_rl2 = mHasRasterCoveragesTables;
    if ( !sLayerName.isEmpty() )
    {
      // Assume a layer is being loaded
      // - will load Spatialite, with RasterLite2, connection , if not done allready
      if ( !dbHasSpatialite( true, true ) )
      {
        return bRc;
      }
      //----------------------------------------------------------
      // Extract TableName/GeometryColumn from sent 'table_name' or 'table_name(field_name)' from LayerName
      SpatialiteDbInfo::parseLayerName( sLayerName, sTableName, sGeometryColumn );
      //----------------------------------------------------------
    }
    else
    {
      // Checking only when loading all Layers
      mHasRasterCoveragesTables = 0;
    }
    // For now, be tolerant for older versions that doen not contain this column.
    QString sCopyright = "'' AS copyright";
    QString subQueryLicense = "'' AS name_license";
    sql = QStringLiteral( "SELECT CASE WHEN (Exists(SELECT rootpage FROM sqlite_master WHERE ((name = 'raster_coverages') AND (sql LIKE '%copyright%'))) = 0) THEN -1 ELSE 0  END" );
    i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL ) && ( sqlite3_column_int( stmt, 0 ) == 0 ) )
        {
          sCopyright = "copyright";
          subQueryLicense = "(SELECT name FROM 'data_licenses' WHERE id=(SELECT license FROM 'raster_coverages')) AS name_license";
        }
      }
      sqlite3_finalize( stmt );
    }
    QString sFields = QString( "coverage_name, title,abstract,%1,%2,srid,extent_minx,extent_miny,extent_maxx,extent_maxy," ).arg( sCopyright ).arg( subQueryLicense );
    sFields += QString( "horz_resolution, vert_resolution, num_bands, tile_width, tile_height," );
    sFields += QString( "sample_type, pixel_type, compression" );
    // SELECT RL2_IsValidRasterStatistics ( 'main' , '1910.alt_berlin_coelln_friedrichsweder' , (SELECT statistics FROM raster_coverages WHERE coverage_name ='1910.alt_berlin_coelln_friedrichsweder')) ;
    // TODO update older RasterLite2 Databases
    // SELECT coverage_name, title,abstract,copyright,srid,extent_minx,extent_miny,extent_maxx,extent_maxy,horz_resolution, vert_resolution, sample_type, pixel_type, num_bands, compression FROM raster_coverages
    // SELECT sample_type, pixel_type, compression FROM raster_coverages
    sql = QStringLiteral( "SELECT %1 FROM raster_coverages" ).arg( sFields );
    if ( !sTableName.isEmpty() )
    {
      sql += QStringLiteral( " WHERE (coverage_name=Lower('%1'))" ).arg( sTableName );
    }
    i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
    // QgsDebugMsgLevel( QString( "-I-> SpatialiteDbInfo::GetRasterLite2RasterLayersInfo[raster] rc[%1] sql[%2]" ).arg( i_rc ).arg( sql), 4 );
    if ( i_rc == SQLITE_OK )
    {
      bRc = true;
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
        {
          sTableName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
          if ( sTableName == sLayerName )
          {
            // We have found what we are looking for, prevent RL1/2 and Topology search
            bFoundLayerName = true;
          }
          // Add only if it does not already exist [using true here, would cause loop]
          if ( !getSpatialiteDbLayer( sTableName, false ) )
          {
            QRect layerBandsTileSize;
            QString sLayerSampleType;
            QString sLayerPixelType;
            QString sLayerCompressionType;
            SpatialiteDbLayer *dbLayer = new SpatialiteDbLayer( this );
            if ( dbLayer )
            {
              dbLayer->mLayerId = mLayersCount++;
              dbLayer->setLayerType( SpatialiteDbInfo::RasterLite2Raster );
              dbLayer->mTableName = sTableName;
              dbLayer->mLayerName = dbLayer->mTableName;
              if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
              {
                sFields = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
                if ( sFields.startsWith( "***" ) )
                {
                  sFields = "";
                }
                dbLayer->mTitle = sFields;
                if ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL )
                {
                  sFields = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 2 ) );
                  if ( sFields.startsWith( "***" ) )
                  {
                    sFields = "";
                  }
                  dbLayer->mAbstract = sFields;
                  if ( sqlite3_column_type( stmt, 3 ) != SQLITE_NULL )
                  {
                    sFields = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 3 ) );
                    if ( sFields.startsWith( "***" ) )
                    {
                      sFields = "";
                    }
                    dbLayer->mCopyright = sFields;
                    if ( sqlite3_column_type( stmt, 4 ) != SQLITE_NULL )
                    {
                      dbLayer->mLicense = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 4 ) );
                    }
                    if ( ( sqlite3_column_type( stmt, 5 ) != SQLITE_NULL ) &&
                         ( sqlite3_column_type( stmt, 6 ) != SQLITE_NULL ) &&
                         ( sqlite3_column_type( stmt, 7 ) != SQLITE_NULL ) &&
                         ( sqlite3_column_type( stmt, 8 ) != SQLITE_NULL ) &&
                         ( sqlite3_column_type( stmt, 9 ) != SQLITE_NULL ) &&
                         ( sqlite3_column_type( stmt, 10 ) != SQLITE_NULL ) &&
                         ( sqlite3_column_type( stmt, 11 ) != SQLITE_NULL ) )
                    {
                      // If valid srid, mIsDbValid will be set to true [srid must be set before LayerExtent]
                      dbLayer->setSrid( sqlite3_column_int( stmt, 5 ) );
                      QgsRectangle layerExtent( sqlite3_column_double( stmt, 6 ), sqlite3_column_double( stmt, 7 ), sqlite3_column_double( stmt, 8 ), sqlite3_column_double( stmt, 9 ) );
                      QgsPointXY layerResolution( sqlite3_column_double( stmt, 10 ), sqlite3_column_double( stmt, 11 ) );
                      dbLayer->setLayerExtent( layerExtent, layerResolution );
                    }
                    if ( ( sqlite3_column_type( stmt, 12 ) != SQLITE_NULL ) &&
                         ( sqlite3_column_type( stmt, 13 ) != SQLITE_NULL ) &&
                         ( sqlite3_column_type( stmt, 14 ) != SQLITE_NULL ) )
                    {
                      // num_bands, not_used, tile_width, tile_height
                      layerBandsTileSize = QRect( sqlite3_column_int( stmt, 12 ), 0, sqlite3_column_int( stmt, 13 ), sqlite3_column_int( stmt, 14 ) );
                    }
                    if ( ( sqlite3_column_type( stmt, 15 ) != SQLITE_NULL ) &&
                         ( sqlite3_column_type( stmt, 16 ) != SQLITE_NULL ) &&
                         ( sqlite3_column_type( stmt, 17 ) != SQLITE_NULL ) )
                    {
                      // sample_type, pixel_type, compression
                      sLayerSampleType = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 15 ) );
                      sLayerPixelType = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 16 ) );
                      sLayerCompressionType = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 17 ) );
                    }
                    dbLayer->setLayerRasterTypesInfo( layerBandsTileSize, sLayerSampleType, sLayerPixelType, sLayerCompressionType );
                  }
                }
              }
            }
            if ( dbLayer->isLayerValid() )
            {
              //--------------------------------
              QMap<QString, QString> layerTypes;
              // Note: Key must be unique
              layerTypes.insert( QString( "%1_sections(%2)" ).arg( dbLayer->mTableName ).arg( "geometry" ), QStringLiteral( "RasterLite2-Sections" ) );
              layerTypes.insert( QString( "%1_levels" ).arg( dbLayer->mTableName ), QStringLiteral( "RasterLite2-Levels" ) );
              layerTypes.insert( QString( "%1_tiles(%2)" ).arg( dbLayer->mTableName ).arg( "geometry" ), QStringLiteral( "RasterLite2-Tiles" ) );
              layerTypes.insert( QString( "%1_tiles_data" ).arg( dbLayer->mTableName ), QStringLiteral( "RasterLite2-Tiles" ) );
              //--------------------------------
              QMap<QString, QString> mapLayers = mNonSpatialTables;
              for ( QMap<QString, QString>::iterator itLayers = mapLayers.begin(); itLayers != mapLayers.end(); ++itLayers )
              {
                // raster_coverages entries are always lower-case
                QString sKey = itLayers.key();
                if ( sKey.toLower().startsWith( dbLayer->mTableName ) )
                {
                  // remove entry with possible upper-case letters that start with the lower-case dbLayer->mTableName
                  layerTypes.insert( itLayers.key(), QStringLiteral( "" ) ); // No entry is desired
                }
              }
              // Remove (and delete) the RasterLite2 Admin tables, which should not be shown [returns amount deleted (not needed)]
              i_rc = removeAdminTables( layerTypes );
              //--------------------------------
              // Resolve unset settings [may become invalid in SpatialiteDbLayer::getCapabilities]
              if ( dbLayer->prepare() )
              {
                if ( dbLayer->mLayerName == sLayerName )
                {
                  // We have found what we are looking for, prevent RL1 and Topology search
                  bFoundLayerName = true;
                }
                // i_removed_count should be 14, otherwise something is wrong
                mDbLayers.insert( dbLayer->mLayerName, dbLayer );
                if ( sLayerName.isEmpty() )
                {
                  mHasRasterCoveragesTables++;
                }
              }
            }
            else
            {
              delete dbLayer;
            }
          }
        }
      }
      sqlite3_finalize( stmt );
      if ( i_check_count_rl2 != mHasRasterCoveragesTables )
      {
        // TODO possible warning that something was invalid
      }
    }
    if ( !sLayerName.isEmpty() )
    {
      if ( bFoundLayerName )
      {
        bRc = bFoundLayerName;
      }
    }
    else
    {
      bRc = isDbValid();;
    }
  }
  else
  {
    mErrors.insert( sLayerName, QString( "SpatialiteDbInfo::GetRasterLite2RasterLayersInfo: called with !mIsDbValid [%1]" ).arg( sLayerName ) );
  }
  return bRc;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::removeAdminTables
//-----------------------------------------------------------------
// Moves Adminstration Tables that contain Geometries from lists
// - so these are not available for rendering
// -> readVectorRasterCoverages
// -> GetRasterLite2RasterLayersInfo
// -> readRasterLite1Layers
// -> GetRasterLite1LayersInfo
// -> GetTopologyLayersInfo
//-----------------------------------------------------------------
int SpatialiteDbInfo::removeAdminTables( QMap<QString, QString> layerTypes )
{
  // Remove (and delete) the Layer-Type Admin tables, which should not be shown
  SpatialiteDbLayer *removeLayer = nullptr;
  int i_removed_count = 0;
  //---------------------------------------------------------------
  // - Key:    LayerName formatted as 'table_name(geometry_name)' or 'table_name'
  // - Value: Group-Name to be displayed in QgsSpatiaLiteTableModel as NonSpatialTables
  //---------------------------------------------------------------
  for ( QMap<QString, QString>::iterator itLayers = layerTypes.begin(); itLayers != layerTypes.end(); ++itLayers )
  {
    removeLayer = static_cast<SpatialiteDbLayer *>( mDbLayers.take( itLayers.key() ) );
    // Note: not all of these entries contain geometries
    if ( removeLayer )
    {
      switch ( removeLayer->getLayerType() )
      {
        case SpatialiteDbInfo::SpatialTable:
          mHasSpatialTables--;
          break;
        case SpatialiteDbInfo::SpatialView:
          mHasSpatialViews--;
          break;
        case SpatialiteDbInfo::VirtualShape:
          mHasVirtualShapes--;
          break;
        default:
          break;
      }
      delete removeLayer;
      i_removed_count++;
    }
    if ( mVectorLayers.contains( itLayers.key() ) )
    {
      // If found, the  mVectorLayers entries must also be removed
      i_removed_count += mVectorLayers.remove( itLayers.key() );
      i_removed_count += mVectorLayersTypes.remove( itLayers.key() );
    }
    // Replace type
    if ( itLayers.value().isEmpty() )
    {
      // No entry is desired, remove
      i_removed_count += mNonSpatialTables.remove( itLayers.key() );
    }
    else
    {
      if ( !mNonSpatialTables.contains( itLayers.key() ) )
      {
        // Classify the entry into a group to be shown in the NonSpatialTables ares
        mNonSpatialTables[ itLayers.key() ] = itLayers.value();
      }
    }
    mHasVectorLayers = mVectorLayers.count();
  }
  return  i_removed_count;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::removeGeoTable
//-----------------------------------------------------------------
// Removes Layers that are contained in a GeoTable that has been removed
// - so these are not available for rendering
// -> dropGeoTable
//-----------------------------------------------------------------
int SpatialiteDbInfo::removeGeoTable( QString sTableName )
{
  // Remove (and delete) the Layer-Type Admin tables, which should not be shown
  SpatialiteDbLayer *removeLayer = nullptr;
  int i_removed_count = 0;
  for ( QMap<QString, QString>::iterator itLayers = mVectorLayers.begin(); itLayers != mVectorLayers.end(); ++itLayers )
  {
    if ( itLayers.key().startsWith( sTableName ) )
    {
      mVectorLayers.remove( itLayers.key() );
      mVectorLayersTypes.remove( itLayers.key() );
      removeLayer = static_cast<SpatialiteDbLayer *>( mDbLayers.take( itLayers.key() ) );
      // Note: not all of these entries contain geometries
      if ( removeLayer )
      {
        switch ( removeLayer->getLayerType() )
        {
          case SpatialiteDbInfo::SpatialTable:
            mHasSpatialTables--;
            break;
          case SpatialiteDbInfo::SpatialView:
            mHasSpatialViews--;
            break;
          case SpatialiteDbInfo::VirtualShape:
            mHasVirtualShapes--;
            break;
          default:
            break;
        }
        delete removeLayer;
        removeLayer = nullptr;
        i_removed_count++;
      }
    }
  }
  mHasVectorLayers = mVectorLayers.count();
  return  i_removed_count;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::GetTopologyLayersInfo
//-----------------------------------------------------------------
// All Layers will be loaded if sLayerName is empty
// - otherwise load only a specific Layer
//-----------------------------------------------------------------
bool SpatialiteDbInfo::GetTopologyLayersInfo( QString sLayerName )
{
  // This is still in a experiental phase
  bool bRc = false;

  if ( isDbValid() )
  {
    sqlite3_stmt *stmt = nullptr;
    sqlite3_stmt *stmtSubquery = nullptr;
    bool bFoundLayerName = false;
    int i_rc = 0;
    QString sTableName  = QString::null;
    QString sGeometryColumn = QString::null;
    int i_check_count_topology = mHasTopologyExportTables;

    if ( !sLayerName.isEmpty() )
    {
      // Assume a layer is being loaded
      // - will load Spatialite connection , if not done allready
      if ( !dbHasSpatialite( true ) )
      {
        return bRc;
      }
      //----------------------------------------------------------
      // Extract TableName/GeometryColumn from sent 'table_name' or 'table_name(field_name)' from LayerName
      SpatialiteDbInfo::parseLayerName( sLayerName, sTableName, sGeometryColumn );
      //----------------------------------------------------------
    }
    else
    {
      // Checking only when loading all Layers
      mHasTopologyExportTables = 0;
    }
    // SELECT topology_name, srid FROM topologies
    QString sFields = QString( "topology_name, srid" );
    QString sql = QStringLiteral( "SELECT %1 FROM topologies" ).arg( sFields );
    if ( !sLayerName.isEmpty() )
    {
      sql += QStringLiteral( " WHERE (topology_name=Lower('%1'))" ).arg( sTableName );
    }
    i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      bRc = true;
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
        {
          QString sTableName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
          if ( sTableName == sLayerName )
          {
            // We have found what we are looking for, prevent RL1/2 and Topology search
            bFoundLayerName = true;
          }
          // Add only if it does not already exist [using true here, would cause loop]
          if ( !getSpatialiteDbLayer( sTableName, false ) )
          {
            SpatialiteDbLayer *dbLayer = new SpatialiteDbLayer( this );
            if ( dbLayer )
            {
              dbLayer->mLayerId = mLayersCount++;
              dbLayer->setLayerType( SpatialiteDbInfo::SpatialiteTopology );
              dbLayer->mTableName = sTableName;
              dbLayer->mLayerName = dbLayer->mTableName;
              if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
              {
                // If valid srid, mIsLayerValid will be set to true
                dbLayer->setSrid( sqlite3_column_int( stmt, 1 ) );
              }
              // Note: Key must be unique
              QMap<QString, QString> layerTypes;
              if ( dbLayer->isLayerValid() )
              {
#if 0
                sFields = QString( "extent_minx,extent_miny,extent_maxx,extent_maxy" );
                sql = QStringLiteral( "SELECT %1 FROM %2" ).arg( sFields ).arg( QString( "%1_face_geoms" ).arg( dbLayer->mTableName ) );
                i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmtSubquery, nullptr );
                if ( i_rc == SQLITE_OK )
                {
                  while ( sqlite3_step( stmtSubquery ) == SQLITE_ROW )
                  {
                    if ( ( sqlite3_column_type( stmtSubquery, 0 ) != SQLITE_NULL ) &&
                         ( sqlite3_column_type( stmtSubquery, 1 ) != SQLITE_NULL ) &&
                         ( sqlite3_column_type( stmtSubquery, 2 ) != SQLITE_NULL ) &&
                         ( sqlite3_column_type( stmtSubquery, 3 ) != SQLITE_NULL ) )
                    {
                      QgsRectangle layerExtent( sqlite3_column_double( stmtSubquery, 0 ), sqlite3_column_double( stmtSubquery, 1 ), sqlite3_column_double( stmtSubquery, 2 ), sqlite3_column_double( stmtSubquery, 3 ) );
                      dbLayer->setLayerExtent( layerExtent );
                      dbLayer->mIsLayerValid = true;
                    }
                  }
                  sqlite3_finalize( stmtSubquery );
                }
#endif
                if ( dbLayer->isLayerValid() )
                {
                  // TODO: valid without layers? could be - ?? just not yet created??
                  sFields = QString( "topolayer_id,topolayer_name" );
                  // "topology_ortsteil_segments_topolayers
                  sql = QStringLiteral( "SELECT %1 FROM %2 ORDER BY topolayer_name" ).arg( sFields ).arg( QString( "%1_topolayers" ).arg( dbLayer->mTableName ) );
                  i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmtSubquery, nullptr );
                  if ( i_rc == SQLITE_OK )
                  {
                    while ( sqlite3_step( stmtSubquery ) == SQLITE_ROW )
                    {
                      int i_topolayer_id = 0;
                      QString s_topolayer_tablename = "";
                      QString s_topolayer_layername = "";
                      if ( ( sqlite3_column_type( stmtSubquery, 0 ) != SQLITE_NULL ) &&
                           ( sqlite3_column_type( stmtSubquery, 1 ) != SQLITE_NULL ) )
                      {
                        // The unique id inside the Topology
                        i_topolayer_id = sqlite3_column_int( stmtSubquery, 0 );
                        // The exported SpatialTable [Topology-Features (Metadata) and Geometry]
                        s_topolayer_tablename = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmtSubquery, 1 ) );
                        // The export-table geometry is always called 'geometry'
                        s_topolayer_layername = QString( "%1(geometry)" ).arg( s_topolayer_tablename );
                        // The Topology-Features (Metadata) - topology_admin_segments_topofeatures_1
                        QString featureName = QString( "%1_topofeatures_%2" ).arg( dbLayer->mTableName ).arg( i_topolayer_id );
                        layerTypes.insert( featureName, QStringLiteral( "Topology-ExportData" ) );
                        dbLayer->mAttributeFields.append( QgsField( s_topolayer_tablename, QVariant::UserType, featureName, i_topolayer_id, dbLayer->mLayerId, dbLayer->mTableName ) );
                        // Move the Export-Table (which a  normal SpatialTable), to the Topology Layer
                        // Why has this not been found??: it is not contained in vector_layers or geometry_columns
                        SpatialiteDbLayer *moveLayer = static_cast<SpatialiteDbLayer *>( mDbLayers.take( s_topolayer_layername ) );
                        if ( moveLayer )
                        {
                          moveLayer->mLayerId = i_topolayer_id;
                          moveLayer->setLayerType( SpatialiteDbInfo::TopologyExport );
                          dbLayer->mTopologyExportLayers.insert( moveLayer->mLayerName, moveLayer );
                          dbLayer->mTopologyExportLayersDataSourceUris.insert( moveLayer->mLayerName, moveLayer->getLayerDataSourceUri() );
                          dbLayer->mNumberFeatures++;
                          layerTypes.insert( moveLayer->mLayerName, QStringLiteral( "" ) );
                        }
                      }
                    }
                    sqlite3_finalize( stmtSubquery );
                  }
                }
              }
              if ( dbLayer->isLayerValid() )
              {
                layerTypes.insert( QString( "%1_edge(%2)" ).arg( dbLayer->mTableName ).arg( "geom" ), QStringLiteral( "Topology-Edge" ) );
                layerTypes.insert( QString( "%1_face(%2)" ).arg( dbLayer->mTableName ).arg( "mbr" ), QStringLiteral( "Topology-Face" ) );
                layerTypes.insert( QString( "%1_node(%2)" ).arg( dbLayer->mTableName ).arg( "geom" ), QStringLiteral( "Topology-Nodes" ) );
                layerTypes.insert( QString( "%1_seeds(%2)" ).arg( dbLayer->mTableName ).arg( "geom" ), QStringLiteral( "Topology-Seeds" ) );
                layerTypes.insert( QString( "%1_edge_seeds(%2)" ).arg( dbLayer->mTableName ).arg( "geom" ), QStringLiteral( "Topology-View-Edge_Seeds" ) );
                layerTypes.insert( QString( "%1_face_seeds(%2)" ).arg( dbLayer->mTableName ).arg( "geom" ), QStringLiteral( "Topology-View-Face_Seeds" ) );
                layerTypes.insert( QString( "%1_face_geoms(%2)" ).arg( dbLayer->mTableName ).arg( "geom" ), QStringLiteral( "Topology-View-Face_Geoms" ) );
                layerTypes.insert( QString( "%1_topolayers" ).arg( dbLayer->mTableName ), QStringLiteral( "Topology-Layers" ) );
                layerTypes.insert( QString( "%1_topofeatures" ).arg( dbLayer->mTableName ), QStringLiteral( "Topology-Features" ) );
                // Remove (and delete) the Topology Admin tables, which should not be shown  [returns amount deleted (not needed)]
                i_rc = removeAdminTables( layerTypes );
                //---------------
                // Resolve unset settings [may become invalid in SpatialiteDbLayer::getCapabilities]
                if ( dbLayer->prepare() )
                {
                  if ( dbLayer->mLayerName == sLayerName )
                  {
                    // We have found what we are looking for, prevent RL1/2 and Topology search
                    bFoundLayerName = true;
                  }

                  mDbLayers.insert( dbLayer->mLayerName, dbLayer );
                  if ( sLayerName.isEmpty() )
                  {
                    mHasTopologyExportTables++;
                  }
                }
              }
              else
              {
                delete dbLayer;
              }
            }
          }
        }
      }
      sqlite3_finalize( stmt );
      if ( i_check_count_topology != mHasTopologyExportTables )
      {
        // TODO possible warning that something was invalid
      }
    }
    if ( !sLayerName.isEmpty() )
    {
      if ( bFoundLayerName )
      {
        bRc = bFoundLayerName;
      }
    }
    else
    {
      bRc = isDbValid();;
    }
  }
  else
  {
    mErrors.insert( sLayerName, QString( "SpatialiteDbInfo::GetTopologyLayersInfo: called with !mIsDValid [%1]" ).arg( sLayerName ) );
  }
  return bRc;
}
//-----------------------------------------------------------------b
// SpatialiteDbInfo::GetRasterLite1LayersInfo
//-----------------------------------------------------------------
// All Layers will be loaded if sLayerName is empty
// - otherwise load only a specific Layer
//-----------------------------------------------------------------
bool SpatialiteDbInfo::GetRasterLite1LayersInfo( QString sLayerName )
{
  bool bRc = false;
  if ( isDbValid() )
  {
    sqlite3_stmt *stmt = nullptr;
    sqlite3_stmt *stmtSubquery = nullptr;
    bool bFoundLayerName = false;
    int i_rc = 0;
    QString sTableName  = QString::null;
    QString sGeometryColumn = QString::null;
    int i_check_count_rl1 = mHasRasterLite1Tables;
    if ( !sLayerName.isEmpty() )
    {
      //----------------------------------------------------------
      // Extract TableName/GeometryColumn from sent 'table_name' or 'table_name(field_name)' from LayerName
      SpatialiteDbInfo::parseLayerName( sLayerName, sTableName, sGeometryColumn );
      //----------------------------------------------------------
    }
    else
    {
      // Checking only when loading all Layers
      mHasRasterLite1Tables = 0;
    }
    QString sFields = QString( "table_name, geometry_column, row_count, extent_min_x, extent_min_y, extent_max_x, extent_max_y" );
    // truemarble-4km.sqlite
    QString sql = QStringLiteral( "SELECT %1 FROM layer_statistics WHERE ((raster_layer=1) OR ((raster_layer=0) AND (table_name LIKE '%_metadata')))" ).arg( sFields );
    if ( !sTableName.isEmpty() )
    {
      sql = QStringLiteral( "SELECT %1 FROM layer_statistics WHERE (((raster_layer=1) AND (table_name = '%2')) OR ((raster_layer=0) AND (table_name = '%2_metadata')))" ).arg( sFields ).arg( sTableName );
    }
    i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
    // QgsDebugMsgLevel( QString( "-I-> SpatialiteDbInfo::GetRasterLite1LayersInfo -1- i_rc[%1] sql[%2]" ).arg( i_rc ).arg( sql ), 3 );
    if ( i_rc == SQLITE_OK )
    {
      bRc = true;
      // TrueMarble_metadata     geometry   195 6.250000  35.252083 18.750000 47.750000
      // srtm_metadata                 geometry 1211  6.249375  35.251208 18.748959 47.750792
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
        {
          sTableName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
          if ( sTableName.contains( "_metadata" ) )
          {
            sTableName.remove( "_metadata" );
          }
          if ( sTableName == sLayerName )
          {
            // We have found what we are looking for, prevent RL1/2 and Topology search
            bFoundLayerName = true;
          }
          // Add only if it does not already exist [using true here, would cause loop]
          if ( !getSpatialiteDbLayer( sTableName, false ) )
          {
            SpatialiteDbLayer *dbLayer = new SpatialiteDbLayer( this );
            if ( dbLayer )
            {
              dbLayer->mLayerId = mLayersCount++;
              dbLayer->setLayerType( SpatialiteDbInfo::RasterLite1 );
              dbLayer->mTableName = sTableName;
              dbLayer->mLayerName = dbLayer->mTableName;
              dbLayer->mCoordDimensions = GAIA_XY;
              QgsRectangle layerExtent;
              if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
              {
                dbLayer->mGeometryColumn = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
              }
              if ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL )
              {
                dbLayer->mNumberFeatures = sqlite3_column_int( stmt, 2 );
              }
              if ( ( sqlite3_column_type( stmt, 3 ) != SQLITE_NULL ) &&
                   ( sqlite3_column_type( stmt, 4 ) != SQLITE_NULL ) &&
                   ( sqlite3_column_type( stmt, 5 ) != SQLITE_NULL ) &&
                   ( sqlite3_column_type( stmt, 6 ) != SQLITE_NULL ) )
              {
                // Note: set extend only after setting the srid (creation of EWKT)
                layerExtent.set( sqlite3_column_double( stmt, 3 ), sqlite3_column_double( stmt, 4 ), sqlite3_column_double( stmt, 5 ), sqlite3_column_double( stmt, 6 ) );
                // TrueMarble_metadata,srtm_metadata
                // -- ---------------------------------- --
                // A valid Raster1-Layer consists of two child-TABLEs
                // -'layer_name'_metadata
                // -> contains a geometry of bitmap in _rasterdata
                // --> first record must contain a valid entry
                // -'layer_name'_rasterdata
                // -> contains a jpeg bitmap [_metadata geometry]
                // --> first record must contain valid data
                // -- ---------------------------------- --
                sql = QStringLiteral( "SELECT " );
                sql += QStringLiteral( "(SELECT Exists(SELECT id FROM '%1' WHERE ((id=0) AND (source_name = 'raster metadata'))))," ).arg( QString( "%1_metadata" ).arg( dbLayer->mTableName ) );
                sql += QStringLiteral( "(SELECT Exists(SELECT id FROM '%1' WHERE (id=1)))" ).arg( QString( "%1_rasters" ).arg( dbLayer->mTableName ) );
                i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmtSubquery, nullptr );
                if ( i_rc == SQLITE_OK )
                {
                  while ( sqlite3_step( stmtSubquery ) == SQLITE_ROW )
                  {
                    if ( ( sqlite3_column_type( stmtSubquery, 0 ) != SQLITE_NULL ) &&
                         ( sqlite3_column_type( stmtSubquery, 1 ) != SQLITE_NULL ) )
                    {
                      if ( ( sqlite3_column_int( stmtSubquery, 0 ) == 1 ) &&
                           ( sqlite3_column_int( stmtSubquery, 1 ) == 1 ) )
                      {
                        // the expected results were found
                        dbLayer->mIsLayerValid = true;
                      }
                    }
                  }
                  sqlite3_finalize( stmtSubquery );
                }
              }
              if ( dbLayer->isLayerValid() )
              {
                QString metadataFields = "f_geometry_column,type,coord_dimension,srid,spatial_index_enabled";
                sql = QStringLiteral( "SELECT %1 FROM 'geometry_columns' WHERE (f_table_name= '%2')" ).arg( metadataFields ).arg( QString( "%1_metadata" ).arg( dbLayer->mTableName ) );
                i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmtSubquery, nullptr );
                if ( i_rc == SQLITE_OK )
                {
                  while ( sqlite3_step( stmtSubquery ) == SQLITE_ROW )
                  {
                    if ( ( sqlite3_column_type( stmtSubquery, 0 ) != SQLITE_NULL ) &&
                         ( sqlite3_column_type( stmtSubquery, 1 ) != SQLITE_NULL ) &&
                         ( sqlite3_column_type( stmtSubquery, 2 ) != SQLITE_NULL ) )
                    {
                      dbLayer->mGeometryColumn =  QString::fromUtf8( ( const char * ) sqlite3_column_text( stmtSubquery, 0 ) );
                      QString geometryType =  QString::fromUtf8( ( const char * ) sqlite3_column_text( stmtSubquery, 1 ) );
                      QString geometryDimension =  QString::fromUtf8( ( const char * ) sqlite3_column_text( stmtSubquery, 2 ) );
                      dbLayer->setGeometryType( SpatialiteDbLayer::GetGeometryTypeLegacy( geometryType, geometryDimension ) );
                    }
                    if ( ( sqlite3_column_type( stmtSubquery, 3 ) != SQLITE_NULL ) &&
                         ( sqlite3_column_type( stmtSubquery, 4 ) != SQLITE_NULL ) )
                    {
                      dbLayer->setSrid( sqlite3_column_int( stmtSubquery, 3 ) );
                      dbLayer->setLayerExtent( layerExtent );
                      dbLayer->setSpatialIndexType( sqlite3_column_int( stmtSubquery, 4 ) );
                    }
                  }
                  sqlite3_finalize( stmtSubquery );
                }
              }
              if ( dbLayer->isLayerValid() )
              {
                //--------------------------------
                // Note: Key must be unique
                QMap<QString, QString> layerTypes;
                layerTypes.insert( QString( "%1_metadata(%2)" ).arg( dbLayer->mTableName ).arg( dbLayer->mGeometryColumn ), QStringLiteral( "RasterLite1-Metadata" ) );
                layerTypes.insert( QString( "%1_raster" ).arg( dbLayer->mTableName ), ( "RasterLite1-Raster" ) );
                // Remove (and delete) the RasterLite1 Admin tables, which should not be shown [returns amount deleted (not needed)]
                i_rc = removeAdminTables( layerTypes );
                if ( mHasLegacyGeometryLayers > 0 )
                {
                  mHasLegacyGeometryLayers = mHasVectorLayers;
                }
                dbLayer->mGeometryColumn = "";
                dbLayer->mLayerName = dbLayer->mTableName;
                dbLayer->setGeometryType( QgsWkbTypes::Unknown );
                // Resolve unset settings [may become invalid in SpatialiteDbLayer::getCapabilities]
                if ( dbLayer->prepare() )
                {
                  mDbLayers.insert( dbLayer->mLayerName, dbLayer );
                  if ( sLayerName.isEmpty() )
                  {
                    mHasRasterLite1Tables++;
                  }
                  if ( dbLayer->mLayerName == sLayerName )
                  {
                    // We have found what we are looking for, prevent RL1/2 and Topology search
                    bFoundLayerName = true;
                  }
                }
              }
              else
              {
                delete dbLayer;
              }
            }
          }
        }
      }
      sqlite3_finalize( stmt );
    }
    if ( i_check_count_rl1 != mHasRasterLite1Tables )
    {
      // TODO (when needed): possible warning that something was invalid
    }
    if ( !sLayerName.isEmpty() )
    {
      if ( bFoundLayerName )
      {
        bRc = bFoundLayerName;
      }
    }
    else
    {
      bRc = isDbValid();;
    }
  }
  else
  {
    mErrors.insert( sLayerName, QString( "SpatialiteDbInfo::GetRasterLite1LayersInfo: called with !mIsDbValid [%1]" ).arg( sLayerName ) );
  }
  return bRc;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::GetMBTilesLayersInfo
//-----------------------------------------------------------------
// All Layers will be loaded if sLayerName is empty
// - otherwise load only a specific Layer
// Note: MBTiles can contain only one Layer
// - programming logic remains the same for unified coding
//-----------------------------------------------------------------
bool SpatialiteDbInfo::GetMBTilesLayersInfo( QString sLayerName )
{
  bool bRc = false;
  if ( isDbValid() )
  {
    sqlite3_stmt *stmt = nullptr;
    // sqlite3_stmt *stmtSubquery = nullptr;
    bool bFoundLayerName = false;
    int i_rc = 0;
    QString sTableName  = QString::null;
    QString sGeometryColumn = QString::null;
    int i_check_count_mbtiles = mHasMBTilesTables;
    if ( !sLayerName.isEmpty() )
    {
      //----------------------------------------------------------
      // Extract TableName/GeometryColumn from sent 'table_name' or 'table_name(field_name)' from LayerName
      SpatialiteDbInfo::parseLayerName( sLayerName, sTableName, sGeometryColumn );
      //----------------------------------------------------------
    }
    else
    {
      // Checking only when loading all Layers
      mHasMBTilesTables = 0;
    }
    QString sql = QStringLiteral( "SELECT " );
    // Mandatory parameters as name in metadata
    sql += QStringLiteral( "(SELECT value FROM metadata WHERE (name='name'))," );
    sql += QStringLiteral( "(SELECT value FROM metadata WHERE (name='description'))," );
    sql += QStringLiteral( "(SELECT value FROM metadata WHERE (name='format'))," );
    sql += QStringLiteral( "(SELECT value FROM metadata WHERE (name='minzoom'))," );
    sql += QStringLiteral( "(SELECT value FROM metadata WHERE (name='maxzoom'))," );
    // 'copyright' may not exist as name in metadata [will be empty if it does not exist]
    sql += QStringLiteral( "(SELECT CASE WHEN (Exists(SELECT value FROM metadata WHERE (name='copyright'))) THEN " );
    sql += QStringLiteral( "(SELECT value FROM metadata WHERE (name='copyright')) ELSE '' END)," );
    // 'license' may not exist as name in metadata [will be empty if it does not exist]
    sql += QStringLiteral( "(SELECT CASE WHEN (Exists(SELECT value FROM metadata WHERE (name='license'))) THEN " );
    sql += QStringLiteral( "(SELECT value FROM metadata WHERE (name='license')) ELSE '' END)," );
    // Mandatory parameter as name in metadata
    sql += QStringLiteral( "(SELECT value FROM metadata WHERE (name='bounds'))," );
    // Determine if view or table base MBTiles Database
    sql += QStringLiteral( "(SELECT count(tbl_name) FROM sqlite_master WHERE ((type = 'table' OR type = 'view') AND (tbl_name IN ('metadata','tiles','images','map'))))" );
    i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      bRc = true;
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
        {
          // Use file-name (excluding the path) as LayerName
          // - metadata name/value may not be unique across different mbtiles-files [should be used only as title]
          QString sTableName = getFileName();
          if ( sTableName == sLayerName )
          {
            // We have found what we are looking for, prevent RL1/2 and Topology search
            bFoundLayerName = true;
          }
          // Add only if it does not already exist [using true here, would cause loop]
          if ( !getSpatialiteDbLayer( sTableName, false ) )
          {
            SpatialiteDbLayer *dbLayer = new SpatialiteDbLayer( this );
            if ( dbLayer )
            {
              dbLayer->mLayerId = mLayersCount++;
              dbLayer->mTableName = sTableName;
              dbLayer->mLayerName = dbLayer->mTableName;
              dbLayer->mTitle = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
              if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
              {
                dbLayer->mAbstract = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
              }
              if ( dbLayer->mAbstract.isEmpty() )
              {
                if ( ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL ) &&
                     ( sqlite3_column_type( stmt, 3 ) != SQLITE_NULL ) &&
                     ( sqlite3_column_type( stmt, 4 ) != SQLITE_NULL ) )
                {
                  dbLayer->mAbstract = QString( "format=%1 ; minzoom=%2 ; maxzoom=%3" ).arg( QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 2 ) ) ).arg( QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 3 ) ) ).arg( QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 4 ) ) );
                }
              }
              if ( sqlite3_column_type( stmt, 5 ) != SQLITE_NULL )
              {
                // 'copyright' may not exist as name in metadata [will be empy if it does not exist]
                dbLayer->mCopyright = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 5 ) );
              }
              if ( sqlite3_column_type( stmt, 6 ) != SQLITE_NULL )
              {
                // 'license' may not exist as name in metadata [will be empty if it does not exist]
                dbLayer->mLicense = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 6 ) );
              }
              if ( ( sqlite3_column_type( stmt, 7 ) != SQLITE_NULL ) &&
                   ( sqlite3_column_type( stmt, 8 ) != SQLITE_NULL ) )
              {
                SpatialiteLayerType type_MBTiles = MBTilesTable;
                if ( sqlite3_column_int( stmt, 8 ) > 2 )
                {
                  type_MBTiles = MBTilesView;
                }
                dbLayer->setLayerType( type_MBTiles );
                // If valid srid, mIsDbValid will be set to true [srid must be set before LayerExtent]
                dbLayer->setSrid( 4326 );
                // Bounds: 11.249999999999993,43.06888777416961,14.062499999999986,44.08758502824516
                QStringList saBounds = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 7 ) ).split( "," );
                if ( saBounds.size() == 4 )
                {
                  QgsRectangle layerExtent( saBounds.at( 0 ).toDouble(), saBounds.at( 1 ).toDouble(), saBounds.at( 2 ).toDouble(), saBounds.at( 3 ).toDouble() );
                  dbLayer->setLayerExtent( layerExtent );
                }
              }
              if ( dbLayer->isLayerValid() )
              {
                // Resolve unset settings [may become invalid in SpatialiteDbLayer::getCapabilities]
                if ( dbLayer->prepare() )
                {
                  if ( dbLayer->mLayerName == sLayerName )
                  {
                    // We have found what we are looking for, prevent RL1/2 and Topology search
                    bFoundLayerName = true;
                  }
                  mDbLayers.insert( dbLayer->mLayerName, dbLayer );
                  if ( sLayerName.isEmpty() )
                  {
                    mHasMBTilesTables++;
                  }
                }
              }
              else
              {
                delete dbLayer;
              }
            }
          }
        }
      }
      sqlite3_finalize( stmt );
    }
    if ( i_check_count_mbtiles != mHasMBTilesTables )
    {
      // TODO possible warning that something was invalid
    }
    if ( !sLayerName.isEmpty() )
    {
      if ( bFoundLayerName )
      {
        bRc = bFoundLayerName;
      }
    }
    else
    {
      bRc = isDbValid();;
    }
  }
  else
  {
    mErrors.insert( sLayerName, QString( "SpatialiteDbInfo::GetMBTilesLayersInfo: called with !mIsDbValid [%1]" ).arg( sLayerName ) );
  }
  return bRc;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::GetGeoPackageLayersInfo
//-----------------------------------------------------------------
// All Layers will be loaded if sLayerName is empty
// - otherwise load only a specific Layer
//-----------------------------------------------------------------
bool SpatialiteDbInfo::GetGeoPackageLayersInfo( QString sLayerName )
{
  bool bRc = false;
  if ( isDbValid() )
  {
    sqlite3_stmt *stmt = nullptr;
    sqlite3_stmt *stmtSubquery = nullptr;
    bool bFoundLayerName = false;
    int i_rc = 0;
    QString sTableName  = QString::null;
    QString sGeometryColumn = QString::null;
    int i_check_count_geopackage = mHasGeoPackageTables;
    if ( !sLayerName.isEmpty() )
    {
      //----------------------------------------------------------
      // Extract TableName/GeometryColumn from sent 'table_name' or 'table_name(field_name)' from LayerName
      SpatialiteDbInfo::parseLayerName( sLayerName, sTableName, sGeometryColumn );
      //----------------------------------------------------------
    }
    else
    {
      // Checking only when loading all Layers
      mHasGeoPackageTables = 0;
      mHasGeoPackageVectors = 0;
      mHasGeoPackageRasters = 0;
    }
    QString sFields = QString( "table_name, data_type, identifier, description, min_x,min_y, max_x, max_y, srs_id" );
    // SELECT table_name, data_type, identifier, description, min_x,min_y, max_x, max_y, srs_id FROM gpkg_contents
    QString sql = QStringLiteral( "SELECT %1 FROM gpkg_contents" ).arg( sFields );
    i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      bRc = true;
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 3 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 4 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 5 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 6 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 7 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 8 ) != SQLITE_NULL ) )
        {
          QString sTable_Name = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
          QString sLayer_Name = sTable_Name;
          QString sLayerType = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
          SpatialiteLayerType type_GeoPackage = GeoPackageVector;
          if ( sLayerType == "tiles" )
          {
            // data_type=tiles|features
            type_GeoPackage = GeoPackageRaster;
            if ( sLayerName.isEmpty() )
            {
              mHasGeoPackageRasters++;
            }
          }
          QString sTitle = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 2 ) );
          QString sAbstract = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 3 ) );
          // min_x,min_y, max_x, max_y
          QgsRectangle layerExtent( sqlite3_column_double( stmt, 4 ), sqlite3_column_double( stmt, 5 ), sqlite3_column_double( stmt, 6 ), sqlite3_column_double( stmt, 7 ) );
          int iSrid = sqlite3_column_int( stmt, 8 );
          QString sGeometryColumn = "";
          QString sGeometryType = "";
          QString sGeometryDimension = "XY";
          int isZ = 0;
          int isM = 0;
          QgsWkbTypes::Type geometryType = QgsWkbTypes::Unknown;
          if ( type_GeoPackage == GeoPackageVector )
          {
            sFields = QString( "column_name, geometry_type_name,z,m" );
            sql = QStringLiteral( "SELECT %1 FROM gpkg_geometry_columns WHERE (table_name='%2')" ).arg( sFields ).arg( sTable_Name );
            i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmtSubquery, nullptr );
            if ( i_rc == SQLITE_OK )
            {
              while ( sqlite3_step( stmtSubquery ) == SQLITE_ROW )
              {
                if ( ( sqlite3_column_type( stmtSubquery, 0 ) != SQLITE_NULL ) &&
                     ( sqlite3_column_type( stmtSubquery, 1 ) != SQLITE_NULL ) &&
                     ( sqlite3_column_type( stmtSubquery, 2 ) != SQLITE_NULL ) &&
                     ( sqlite3_column_type( stmtSubquery, 3 ) != SQLITE_NULL ) )
                {
                  sGeometryColumn = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmtSubquery, 0 ) );
                  sGeometryType = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmtSubquery, 1 ) );
                  isZ = sqlite3_column_int( stmtSubquery, 2 ) ;
                  if ( isZ == 1 )
                  {
                    sGeometryDimension = QString( "%1%2" ).arg( sGeometryDimension ).arg( "Z" );
                  }
                  isM = sqlite3_column_int( stmtSubquery, 3 );
                  if ( isM == 1 )
                  {
                    sGeometryDimension = QString( "%1%2" ).arg( sGeometryDimension ).arg( "M" );
                  }
                  if ( !sGeometryColumn.isEmpty() )
                  {
                    sLayer_Name = QString( "%1(%2)" ).arg( sTable_Name ).arg( sGeometryColumn );
                  }
                  geometryType = SpatialiteDbLayer::GetGeometryTypeLegacy( sGeometryType, sGeometryDimension );
                  if ( sLayerName.isEmpty() )
                  {
                    mHasGeoPackageVectors++;
                  }
                }
              }
              sqlite3_finalize( stmtSubquery );
            }
          }
          // Add only if it does not already exist [using true here, would cause loop]
          if ( !getSpatialiteDbLayer( sLayer_Name, false ) )
          {
            SpatialiteDbLayer *dbLayer = new SpatialiteDbLayer( this );
            if ( dbLayer )
            {
              dbLayer->mLayerId = mLayersCount++;
              dbLayer->mTableName = sTable_Name;
              dbLayer->mLayerName = sLayer_Name;
              dbLayer->mTitle = sTitle;
              dbLayer->mAbstract = sAbstract;
              dbLayer->setLayerType( type_GeoPackage );
              // If valid srid, mIsDbValid will be set to true [srid must be set before LayerExtent]
              dbLayer->setSrid( iSrid );
              dbLayer->setLayerExtent( layerExtent );
              dbLayer->mGeometryColumn = sGeometryColumn;
              dbLayer->setGeometryType( geometryType );
              if ( dbLayer->isLayerValid() )
              {
                // Resolve unset settings [may become invalid in SpatialiteDbLayer::getCapabilities]
                if ( dbLayer->prepare() )
                {
                  if ( dbLayer->mLayerName == sLayerName )
                  {
                    // We have found what we are looking for, prevent RL1/2 and Topology search
                    bFoundLayerName = true;
                  }
                  mDbLayers.insert( dbLayer->mLayerName, dbLayer );
                  if ( sLayerName.isEmpty() )
                  {
                    mHasGeoPackageTables++;
                  }
                }
              }
              else
              {
                delete dbLayer;
              }
            }
          }
          else
          {
            if ( sLayer_Name == sLayerName )
            {
              // We have found what we are looking for, prevent RL1/2 and Topology search
              bFoundLayerName = true;
            }
          }
        }
      }
      sqlite3_finalize( stmt );
    }
    if ( i_check_count_geopackage != mHasGeoPackageTables )
    {
      // TODO possible warning that something was invalid
    }
    if ( !sLayerName.isEmpty() )
    {
      if ( bFoundLayerName )
      {
        bRc = bFoundLayerName;
      }
    }
    else
    {
      bRc = isDbValid();;
    }
  }
  else
  {
    mErrors.insert( sLayerName, QString( "SpatialiteDbInfo::GetGeoPackageLayersInfo: called with !mIsDbValid [%1]" ).arg( sLayerName ) );
  }
  return bRc;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::GetFdoOgrLayersInfo
//-----------------------------------------------------------------
// All Layers will be loaded if sLayerName is empty
// - otherwise load only a specific Layer
//-----------------------------------------------------------------
bool SpatialiteDbInfo::GetFdoOgrLayersInfo( QString sLayerName )
{
  bool bRc = false;
  if ( isDbValid() )
  {
    int i_check_count_fdoogr = mHasFdoOgrTables;
    sqlite3_stmt *stmt = nullptr;
    sqlite3_stmt *stmtSubquery = nullptr;
    bool bFoundLayerName = false;
    int i_rc = 0;
    QString sTableName  = QString::null;
    QString sGeometryColumn = QString::null;
    if ( !sLayerName.isEmpty() )
    {
      //----------------------------------------------------------
      // Extract TableName/GeometryColumn from sent 'table_name' or 'table_name(field_name)' from LayerName
      SpatialiteDbInfo::parseLayerName( sLayerName, sTableName, sGeometryColumn );
      //----------------------------------------------------------
    }
    else
    {
      // Checking only when loading all Layers
      mHasFdoOgrTables = 0;
    }
    QString sFields = QString( "f_table_name, f_geometry_column, geometry_type, coord_dimension, srid,geometry_format" );
    // SELECT f_table_name, f_geometry_column, geometry_type, coord_dimension, srid,geometry_format FROM geometry_columns
    QString sql = QStringLiteral( "SELECT %1 FROM geometry_columns" ).arg( sFields );
    i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      bRc = true;
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 3 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 4 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 5 ) != SQLITE_NULL ) )
        {
          QString sTable_Name = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
          QString sLayer_Name = sTable_Name;
          QString sGeometry_Column = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
          QgsWkbTypes::Type geometryType = SpatialiteDbLayer::GetGeometryType( sqlite3_column_int( stmt, 2 ), sqlite3_column_int( stmt, 3 ) );
          int iSrid = sqlite3_column_int( stmt, 4 );
          if ( !sGeometry_Column.isEmpty() )
          {
            sLayer_Name = QString( "%1(%2)" ).arg( sTable_Name ).arg( sGeometry_Column );
          }
          QString sGeometryFormat = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 5 ) );
          if ( sGeometryFormat == QStringLiteral( "WKB" ) )
          {
            QgsRectangle layerExtent;
            // SELECT ST_MinX(ST_Collect(ST_GeomFromWKB(GEOMETRY,3035))) AS min_x, ST_MinY(ST_Collect(ST_GeomFromWKB(GEOMETRY,3035)))  AS min_y,ST_MaxX(ST_Collect(ST_GeomFromWKB(GEOMETRY,3035))) AS max_x, ST_MaxY(ST_Collect(ST_GeomFromWKB(GEOMETRY,3035))) AS max_y FROM middle_earth_linestrings;
            sFields = QString( "ST_MinX(ST_Collect(ST_GeomFromWKB(%1,%2))), ST_MinY(ST_Collect(ST_GeomFromWKB(%1,%2))),ST_MaxX(ST_Collect(ST_GeomFromWKB(%1,%2))), ST_MaxY(ST_Collect(ST_GeomFromWKB(%1,%2)))" ).arg( sGeometry_Column ).arg( iSrid );
            sql = QStringLiteral( "SELECT %1 FROM '%2'" ).arg( sFields ).arg( sTable_Name );
            i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmtSubquery, nullptr );
            if ( i_rc == SQLITE_OK )
            {
              while ( sqlite3_step( stmtSubquery ) == SQLITE_ROW )
              {
                if ( ( sqlite3_column_type( stmtSubquery, 0 ) != SQLITE_NULL ) &&
                     ( sqlite3_column_type( stmtSubquery, 1 ) != SQLITE_NULL ) &&
                     ( sqlite3_column_type( stmtSubquery, 2 ) != SQLITE_NULL ) &&
                     ( sqlite3_column_type( stmtSubquery, 3 ) != SQLITE_NULL ) )
                {
                  layerExtent = QgsRectangle( sqlite3_column_double( stmtSubquery, 0 ), sqlite3_column_double( stmtSubquery, 1 ), sqlite3_column_double( stmtSubquery, 2 ), sqlite3_column_double( stmtSubquery, 3 ) );
                }
              }
              sqlite3_finalize( stmtSubquery );
              // Add only if it does not already exist [using true here, would cause loop]
              if ( !getSpatialiteDbLayer( sLayer_Name, false ) )
              {
                SpatialiteDbLayer *dbLayer = new SpatialiteDbLayer( this );
                if ( dbLayer )
                {
                  dbLayer->mLayerId = mLayersCount++;
                  dbLayer->mTableName = sTable_Name;
                  dbLayer->mLayerName = sLayer_Name;
                  dbLayer->setLayerType( GdalFdoOgr );
                  // If valid srid, mIsDbValid will be set to true [srid must be set before LayerExtent]
                  dbLayer->setSrid( iSrid );
                  dbLayer->setLayerExtent( layerExtent );
                  dbLayer->mGeometryColumn = sGeometry_Column;
                  dbLayer->setGeometryType( geometryType );
                  if ( dbLayer->isLayerValid() )
                  {
                    // Resolve unset settings [may become invalid in SpatialiteDbLayer::getCapabilities]
                    if ( dbLayer->prepare() )
                    {
                      if ( dbLayer->mLayerName == sLayerName )
                      {
                        // We have found what we are looking for, prevent RL1/2 and Topology search
                        bFoundLayerName = true;
                      }
                      mDbLayers.insert( dbLayer->mLayerName, dbLayer );
                      if ( sLayerName.isEmpty() )
                      {
                        mHasFdoOgrTables++;
                      }
                    }
                  }
                  else
                  {
                    delete dbLayer;
                  }
                }
              }
            }
          }
        }
      }
      sqlite3_finalize( stmt );
    }
    if ( i_check_count_fdoogr != mHasFdoOgrTables )
    {
      // TODO possible warning that something was invalid
    }
    if ( !sLayerName.isEmpty() )
    {
      if ( bFoundLayerName )
      {
        bRc = bFoundLayerName;
      }
    }
    else
    {
      bRc = isDbValid();
    }
  }
  else
  {
    mErrors.insert( sLayerName, QString( "SpatialiteDbInfo::GetFdoOgrLayersInfo: called with !mIsDbValid [%1]" ).arg( sLayerName ) );
  }
  return bRc;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::readNonSpatialTables
//-----------------------------------------------------------------
bool SpatialiteDbInfo::readNonSpatialTables()
{
  bool bRc = false;
  mNonSpatialTables.clear();
  sqlite3_stmt *stmt = nullptr;
  QString sWhere = QString( "WHERE ((type IN ('table','view','trigger'))" );
  if ( isDbSpatialite() )
  {
    sWhere += QString( " AND (name NOT IN (SELECT table_name FROM vector_layers)))" );
  }
  else
  {
    if ( mHasGeoPackageTables > 0 )
    {
      sWhere += QString( " AND (name NOT IN (SELECT table_name FROM gpkg_contents)))" );
    }
    else if ( mHasFdoOgrTables > 0 )
    {
      sWhere += QString( " AND (name NOT IN (SELECT f_table_name FROM geometry_columns)))" );
    }
    else
    {
      sWhere += QString( ")" );
    }
  }
  QString sql = QStringLiteral( "SELECT name,type FROM sqlite_master %1 ORDER BY  type,name;" ).arg( sWhere );
  // SELECT name,type FROM sqlite_master WHERE ((type IN ('table','view','trigger')) AND (name NOT LIKE 'idx_%') AND (name NOT IN (SELECT table_name FROM vector_layers)))
  // SELECT name,type FROM sqlite_master WHERE ((type IN ('table','view','trigger')) AND (name NOT IN (SELECT table_name FROM vector_layers)))
  int i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
  if ( i_rc == SQLITE_OK )
  {
    bRc = true;
    QMap<QString, QString> spatialiteTypes;
    spatialiteTypes.insert( "SpatialIndex", "Virtual-Internal" );
    spatialiteTypes.insert( "KNN", "Virtual-Internal" );
    spatialiteTypes.insert( "ElementaryGeometries", "Virtual-Internal" );
    spatialiteTypes.insert( "spatialite_history", "Spatialite-Internal" );
    spatialiteTypes.insert( "data_licenses", "Spatialite-Internal" );
    spatialiteTypes.insert( "networks", "Networks-Internal" );
    spatialiteTypes.insert( "geometry_columns", "Geometries-Internal" );
    spatialiteTypes.insert( "sql_statements_log", "Spatialite-Internal" );
    spatialiteTypes.insert( "geom_cols_ref_sys", "Geometries-Internal" );
    spatialiteTypes.insert( "geometry_columns_auth", "Geometries-Internal" );
    spatialiteTypes.insert( "geometry_columns_field_infos", "Geometries-Internal" );
    spatialiteTypes.insert( "geometry_columns_statistics", "Geometries-Internal" );
    spatialiteTypes.insert( "geometry_columns_time", "Geometries-Internal" );
    spatialiteTypes.insert( "vector_layers", "SpatialTable-Internal" );
    spatialiteTypes.insert( "vector_layers_auth", "SpatialTable-Internal" );
    spatialiteTypes.insert( "vector_layers_field_infos", "SpatialTable-Internal" );
    spatialiteTypes.insert( "vector_layers_statistics", "SpatialTable-Internal" );
    spatialiteTypes.insert( "views_geometry_columns", "SpatialView-Internal" );
    spatialiteTypes.insert( "views_geometry_columns_auth", "SpatialView-Internal" );
    spatialiteTypes.insert( "views_geometry_columns_field_infos", "SpatialView-Internal" );
    spatialiteTypes.insert( "views_geometry_columns_statistics", "SpatialView-Internal" );
    spatialiteTypes.insert( "virts_geometry_columns", "VirtualShapes-Internal" );
    spatialiteTypes.insert( "virts_geometry_columns_auth", "VirtualShapes-Internal" );
    spatialiteTypes.insert( "virts_geometry_columns_field_infos", "VirtualShapes-Internal" );
    spatialiteTypes.insert( "virts_geometry_columns_statistics", "VirtualShapes-Internal" );
    spatialiteTypes.insert( "vector_coverages", "RasterLite2-Internal" );
    spatialiteTypes.insert( "vector_coverages_keyword", "RasterLite2-Internal" );
    spatialiteTypes.insert( "vector_coverages_ref_sys", "RasterLite2-Internal" );
    spatialiteTypes.insert( "vector_coverages_srid", "RasterLite2-Internal" );
    spatialiteTypes.insert( "raster_coverages", "RasterLite2-Internal" );
    spatialiteTypes.insert( "raster_coverages_keyword", "RasterLite2-Internal" );
    spatialiteTypes.insert( "raster_coverages_ref_sys", "RasterLite2-Internal" );
    spatialiteTypes.insert( "raster_coverages_srid", "RasterLite2-Internal" );
    spatialiteTypes.insert( "topologies", "Topology-Internal" );
    spatialiteTypes.insert( "layer_statistics", "Spatialite2-Internal" );
    spatialiteTypes.insert( "raster_pyramids", "RasterLite1-Internal" );
    spatialiteTypes.insert( "sqlite_sequence", "Sqlite3-Internal" );
    spatialiteTypes.insert( "sqlite_stat1", "Sqlite3-ANALYZE" );
    spatialiteTypes.insert( "sqlite_stat2", "Sqlite3-identify" );
    spatialiteTypes.insert( "sqlite_stat3", "Sqlite3-ANALYZE" );
    spatialiteTypes.insert( "sqlite_stat4", "Sqlite3-ANALYZE" );
    spatialiteTypes.insert( "spatial_ref_sys", "EPSG-Table" );
    spatialiteTypes.insert( "spatial_ref_sys_all", "EPSG-Internal" );
    spatialiteTypes.insert( "spatial_ref_sys_aux", "EPSG-Internal" );
    // MBTiles specific
    spatialiteTypes.insert( "metadata", "MBTiles-Admin" );
    spatialiteTypes.insert( "tiles", "MBTiles-Raster" );
    spatialiteTypes.insert( "map", "MBTiles-Raster" );
    spatialiteTypes.insert( "images", "MBTiles-Raster" );
    spatialiteTypes.insert( "grids", "MBTiles-Vector" );
    spatialiteTypes.insert( "grid_data", "MBTiles-Vector" );
    spatialiteTypes.insert( "grid_data", "MBTiles-Vector" );
    spatialiteTypes.insert( "grid_key", "MBTiles-Vector" );
    spatialiteTypes.insert( "grid_utfgrid", "MBTiles-Vector" );
    spatialiteTypes.insert( "keymap", "MBTiles-Vector" );
    // GeoPackage specific
    spatialiteTypes.insert( "gpkg_contents", "GeoPackage-Admin" );
    spatialiteTypes.insert( "gpkg_spatial_ref_sys", "EPSG-Table" );
    spatialiteTypes.insert( "gpkg_data_columns", "GeoPackage-Internal" );
    spatialiteTypes.insert( "gpkg_data_column_constraints", "GeoPackage-Internal" );
    spatialiteTypes.insert( "gpkg_extensions", "GeoPackage-Internal" );
    spatialiteTypes.insert( "gpkg_metadata", "GeoPackage-Internal" );
    spatialiteTypes.insert( "gpkg_metadata_reference", "GeoPackage-Internal" );
    spatialiteTypes.insert( "gpkg_geometry_columns", "Geometries-Internal" );
    spatialiteTypes.insert( "gpkg_tile_matrix", "GeoPackage-Tiles" );
    spatialiteTypes.insert( "gpkg_tile_matrix_set", "GeoPackage-Tiles" );
    while ( sqlite3_step( stmt ) == SQLITE_ROW )
    {
      if ( ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )  &&
           ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL ) )
      {
        QString sName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
        QString sType = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
        QString masterType = "Unknown-Data";
        if ( !spatialiteTypes.value( sName ).isEmpty() )
        {
          masterType = spatialiteTypes.value( sName );
        }
        else
        {
          if ( sName.startsWith( "SE_" ) )
          {
            masterType = "Styles-Internal";
          }
          else if ( sName.startsWith( "idx_" ) )
          {
            masterType = "SpatialIndex-Data";
          }
          else if ( sName.startsWith( "rtree_" ) )
          {
            masterType = "SpatialIndex-Data";
          }
          else
          {
            if ( sType == "table" )
            {
              masterType = "Table-Data";
            }
            else if ( sType == "view" )
            {
              masterType = "View-Data";
            }
            else if ( sType == "trigger" )
            {
              masterType = "Trigger-Data";
            }
          }
        }
        if ( mNonSpatialTables.value( sName ).isEmpty() )
        {
          // sName is not contained in the Key-Field (returns value field)
          mNonSpatialTables.insert( sName, masterType );
        }
      }
    }
    sqlite3_finalize( stmt );
  }
  return bRc;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::getDbStyleNamedLayerElement
//-----------------------------------------------------------------
// Retrieval of Style by SpatialiteDbLayer
// or initial reading from readVectorRasterStyles
// checking is done to insure that the Style is usable for QGis
// QGis needs a 'StyledLayerDescriptor' wrapper around the xml
// Spatialite creates only one 'FeatureTypeStyle' by default
// A 'StyledLayerDescriptor' wrapper will be
// - created and returned when missing
//-----------------------------------------------------------------
// If no spatialite Connection existed when first read
//  by readVectorRasterStyles [default], but durring retrievel
//  exists, then readVectorRasterStyles will be called again since
// Spatialite specific commands are needed to retrieve the xml
//-----------------------------------------------------------------
QDomElement SpatialiteDbInfo::getDbStyleNamedLayerElement( SpatialiteDbInfo::SpatialiteLayerType styleType, int iStyleId, QString &errorMessage, int *iDebug, QString sSaveXmlFileName )
{
  QDomElement namedLayerElement;
  int i_loaded_styles = mVectorStyleData.count() + mRasterStyleData.count();
  if ( mLayerNamesStyles.count() > 0 )
  {
    // Prevent a loop, since 'readVectorRasterStyles' will call the function to test the vality of the xml
    if ( i_loaded_styles == 0 )
    {
      // Spatialite driver support was not active during getSniffReadLayers
      // - now that there is a connection and a style is being demanded, load the styles
      bool bTestStylesForQgis = true;
      // Force a Spatialite connection, if needed
      if ( dbHasSpatialite() )
      {
        readVectorRasterStyles( bTestStylesForQgis );
      }
      i_loaded_styles = mVectorStyleData.count() + mRasterStyleData.count();
    }
  }
  QString sStyleXml = QString::null;
  switch ( styleType )
  {
    case SpatialiteDbInfo::StyleVector:
    {
      sStyleXml = getDbVectorStyleXml( iStyleId );
    }
    break;
    case SpatialiteDbInfo::StyleRaster:
    {
      sStyleXml = getDbRasterStyleXml( iStyleId );
    }
    break;
    default:
      return namedLayerElement;
      break;
  }
  if ( !sStyleXml.isEmpty() )
  {
    QDomDocument styleDocument;
    QDomElement styledLayerDescriptor;
    QDomElement featureTypeStyle;
    // If namespaceProcessing is true, the parser recognizes namespaces in the XML file and sets the prefix name, local name and namespace URI to appropriate values.
    bool bNamespaceProcessing = true;
    QString sStyleName;
    QString errorMsg;
    // location of problem associated with errorMsg
    int line, column;
    if ( styleDocument.setContent( sStyleXml, bNamespaceProcessing, &errorMsg, &line, &column ) )
    {
      // check for root SLD element
      styledLayerDescriptor = styleDocument.firstChildElement( QStringLiteral( "StyledLayerDescriptor" ) );
      if ( !styledLayerDescriptor.isNull() )
      {
        // now get the style node out and pass it over to the layer to deserialise...
        namedLayerElement = styledLayerDescriptor.firstChildElement( QStringLiteral( "NamedLayer" ) );
        if ( !namedLayerElement.isNull() )
        {
          if ( ( iDebug ) && ( *iDebug == 1 ) )
          {
            *iDebug = testNamedLayerElement( namedLayerElement,  errorMessage );
          }
          return namedLayerElement;
        }
        else
        {
          errorMessage = QString( " NamedLayer[%1]: not found in StyledLayerDescriptor" ).arg( styledLayerDescriptor.tagName() );
        }
      }
      else
      {
        // As of 2017-08-07: this is the Root-Element stored, but QgsVectorLayer::readSld expects StyledLayerDescriptor
        // Note: an external source may not be using the 'se:' prefix.
        featureTypeStyle = styleDocument.firstChildElement( QStringLiteral( "FeatureTypeStyle" ) );
        QDomElement nameElement_retrieve = featureTypeStyle.firstChildElement( QStringLiteral( "Name" ) );
        sStyleName =  nameElement_retrieve.text();
        QDomDocument rebuildDocument = QDomDocument();
        QDomNode header = rebuildDocument.createProcessingInstruction( QStringLiteral( "xml" ), QStringLiteral( "version=\"1.0\" encoding=\"UTF-8\"" ) );
        rebuildDocument.appendChild( header );
        // Create the root element
        styledLayerDescriptor = rebuildDocument.createElementNS( QStringLiteral( "http://www.opengis.net/sld" ), QStringLiteral( "StyledLayerDescriptor" ) );
        styledLayerDescriptor.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.1.0" ) );
        styledLayerDescriptor.setAttribute( QStringLiteral( "xsi:schemaLocation" ), QStringLiteral( "http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd" ) );
        styledLayerDescriptor.setAttribute( QStringLiteral( "xmlns:ogc" ), QStringLiteral( "http://www.opengis.net/ogc" ) );
        styledLayerDescriptor.setAttribute( QStringLiteral( "xmlns:se" ), QStringLiteral( "http://www.opengis.net/se" ) );
        styledLayerDescriptor.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
        styledLayerDescriptor.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
        rebuildDocument.appendChild( styledLayerDescriptor );
        // Create the NamedLayer element
        namedLayerElement = rebuildDocument.createElement( QStringLiteral( "NamedLayer" ) );
        styledLayerDescriptor.appendChild( namedLayerElement );
        QDomElement nameElement = rebuildDocument.createElement( QStringLiteral( "Name" ) );
        nameElement.appendChild( rebuildDocument.createTextNode( sStyleName ) );
        namedLayerElement.appendChild( nameElement );
        QDomElement userStyleElement = rebuildDocument.createElement( QStringLiteral( "UserStyle" ) );
        QDomElement nameElementStyle = rebuildDocument.createElement( QStringLiteral( "Name" ) );
        nameElementStyle.appendChild( rebuildDocument.createTextNode( sStyleName ) );
        userStyleElement.appendChild( nameElementStyle );
        userStyleElement.appendChild( featureTypeStyle );
        namedLayerElement.appendChild( userStyleElement );
        // check for root SLD element
        styledLayerDescriptor = rebuildDocument.firstChildElement( QStringLiteral( "StyledLayerDescriptor" ) );
        if ( !styledLayerDescriptor.isNull() )
        {
          // now get the style node out and pass it over to the layer to deserialise...
          namedLayerElement = styledLayerDescriptor.firstChildElement( QStringLiteral( "NamedLayer" ) );
          if ( !namedLayerElement.isNull() )
          {
            if ( ( iDebug ) && ( *iDebug == 1 ) )
            {
              if ( !sSaveXmlFileName.isEmpty() )
              {
                QFile xmlFile( sSaveXmlFileName );
                if ( xmlFile.open( QFile::WriteOnly | QFile::Truncate | QIODevice::Text ) )
                {
                  QTextStream xmlTextStream( &xmlFile );
                  rebuildDocument.save( xmlTextStream, 1 );
                  xmlFile.close();
                }
              }
              *iDebug = testNamedLayerElement( namedLayerElement,  errorMessage );
            }
            return namedLayerElement;
          }
        }
      }
    }
    else
    {
      errorMessage = tr( "%1: %2 at line %3 column %4" ).arg( sStyleName ).arg( errorMsg ).arg( line ).arg( column );
    }
  }
  return namedLayerElement;
}
//-----------------------------------------------------------------
// SpatialiteDbInfo::testNamedLayerElement
//-----------------------------------------------------------------
int SpatialiteDbInfo::testNamedLayerElement( QDomElement namedLayerElement,  QString &errorMessage )
{
  int i_rc = 0;
  QString rendererType = QStringLiteral( "singleSymbol" );
  QStringList saLayers;
  QString sNodeName;
  if ( namedLayerElement.tagName() == "NamedLayer" )
  {
    QDomElement testFirstElement;
    // QgsVectorLayer::readSld: get the Name element
    QDomElement nameElement = namedLayerElement.firstChildElement( QStringLiteral( "Name" ) );
    if ( !nameElement.isNull() )
    {
      // QgsFeatureRenderer::loadSld receives namedLayerElement
      // get the UserStyle element
      QDomElement userStyleElement = namedLayerElement.firstChildElement( QStringLiteral( "UserStyle" ) );
      if ( !userStyleElement.isNull() )
      {
        QDomElement nameStyleElement = userStyleElement.firstChildElement( QStringLiteral( "Name" ) );
        // get the FeatureTypeStyle element
        QDomElement featTypeStyleElement = userStyleElement.firstChildElement( QStringLiteral( "FeatureTypeStyle" ) );
        if ( !featTypeStyleElement.isNull() )
        {
          QDomElement nameFeatureTypeStyleElement = featTypeStyleElement.firstChildElement( QStringLiteral( "Name" ) );
          // get the FeatureTypeStyle element
          QDomElement ruleElement = featTypeStyleElement.firstChildElement( QStringLiteral( "Rule" ) );
          QDomElement nameRuleElement = ruleElement.firstChildElement( QStringLiteral( "Name" ) );
          if ( !ruleElement.isNull() )
          {
            while ( !ruleElement.isNull() )
            {
              QDomElement ruleChildElement = ruleElement.firstChildElement();
              while ( !ruleChildElement.isNull() )
              {
                sNodeName = ruleChildElement.localName();
                if ( sNodeName.isEmpty() )
                {
                  // Note: an external source may not be using the 'se:' prefix, thus localName() will be empty.
                  sNodeName = ruleChildElement.tagName();
                }
                // rule has filter or min/max scale denominator, use the RuleRenderer
                if ( sNodeName == QLatin1String( "Filter" ) ||
                     sNodeName == QLatin1String( "MinScaleDenominator" ) ||
                     sNodeName == QLatin1String( "MaxScaleDenominator" ) )
                {
                  QgsDebugMsgLevel( QString( "Filter or Min/MaxScaleDenominator element found NodeName[%1]: need a RuleRenderer" ).arg( sNodeName ), 3 );
                  rendererType = QStringLiteral( "RuleRenderer" );
                  break;
                }
                ruleChildElement = ruleChildElement.nextSiblingElement();
              }
              ruleElement = ruleElement.nextSiblingElement( QStringLiteral( "Rule" ) );
            }
            // create the renderer and return it
            // QgsFeatureRenderer *r = m->createRendererFromSld( featTypeStyleElem, geomType );
            if ( rendererType == "singleSymbol" )
            {
              // QgsFeatureRenderer *QgsSingleSymbolRenderer::createFromSld
              // sending featTypeStyleElement
              // retrieve the Rule element child nodes
              // Checking for the existence of a rule was done above [i_rc = 4]
              QDomElement ruleElement = featTypeStyleElement.firstChildElement( QStringLiteral( "Rule" ) );
              if ( !ruleElement.isNull() )
              {
                QDomElement nameRuleElement = ruleElement.firstChildElement( QStringLiteral( "Name" ) );
                QDomElement ruleChildElement = ruleElement.firstChildElement();
                QDomElement nameRuleChildElement = ruleChildElement.firstChildElement( QStringLiteral( "Name" ) );
                if ( !ruleChildElement.isNull() )
                {
                  while ( !ruleChildElement.isNull() )
                  {
                    sNodeName = ruleChildElement.localName();
                    if ( sNodeName.isEmpty() )
                    {
                      // Note: an external source may not be using the 'se:' prefix, thus localName() will be empty.
                      sNodeName = ruleChildElement.tagName();
                    }
                    if ( sNodeName.endsWith( QLatin1String( "Symbolizer" ) ) )
                    {
                      // create symbol layers for this symbolizer
                      // QgsSymbolLayerUtils::createSymbolLayerListFromSld( childElem, geomType, layers );
                      QString symbolizerName = ruleChildElement.nextSiblingElement().localName();
                      if ( symbolizerName.isEmpty() )
                      {
                        // Note: an external source may not be using the 'se:' prefix, thus localName() will be empty.
                        symbolizerName = ruleChildElement.nextSiblingElement().tagName();
                      }
                      if ( symbolizerName.isEmpty() )
                      {
                        // Note: not sure why this is so, otherwise when only one exist it fails.
                        symbolizerName = sNodeName;
                      }
                      if ( symbolizerName == QLatin1String( "PointSymbolizer" ) )
                      {
                        // first check for Graphic element, nothing will be rendered if not found
                        QDomElement graphicElement = ruleChildElement.firstChildElement( QStringLiteral( "Graphic" ) );
                        if ( graphicElement.isNull() )
                        {
                          i_rc = 110;
                          errorMessage = QStringLiteral( "-E-> During symbolizerName[%1.%2] no 'Graphic' Element in 'Symbolizer-Layers' was found.  ; rc=%3" ).arg( rendererType ).arg( symbolizerName ).arg( i_rc );
                        }
                        else
                        {
                          errorMessage = QStringLiteral( "-I-> symbolizerName[%1.%2] all Elements in 'Symbolizer-Layers' were found.  ; rc=%3" ).arg( rendererType ).arg( symbolizerName ).arg( i_rc );
                          saLayers.append( symbolizerName );
                        }
                      }
                      else if ( symbolizerName == QLatin1String( "LineSymbolizer" ) )
                      {
                        // first check for Graphic element, nothing will be rendered if not found
                        QDomElement strokeElement = ruleChildElement.firstChildElement( QStringLiteral( "Stroke" ) );
                        if ( strokeElement.isNull() )
                        {
                          i_rc = 120;
                          errorMessage = QStringLiteral( "-E-> During symbolizerName[%1.%2] no 'Stroke' Element in 'Symbolizer-Layers' was found.  ; rc=%3" ).arg( rendererType ).arg( symbolizerName ).arg( i_rc );
                        }
                        else
                        {
                          saLayers.append( symbolizerName );
                          errorMessage = QStringLiteral( "-I-> symbolizerName[%1.%2] all Elements in 'Symbolizer-Layers' were found.  ; rc=%3" ).arg( rendererType ).arg( symbolizerName ).arg( i_rc );
                        }
                      }
                      else if ( symbolizerName == QLatin1String( "PolygonSymbolizer" ) )
                      {
                        // get Fill and Stroke elements, nothing will be rendered if both are missing
                        QDomElement fillElement = ruleChildElement.firstChildElement( QStringLiteral( "Fill" ) );
                        QDomElement strokeElement = ruleChildElement.firstChildElement( QStringLiteral( "Stroke" ) );
                        if ( fillElement.isNull() || strokeElement.isNull() )
                        {
                          if ( fillElement.isNull() && strokeElement.isNull() )
                          {
                            i_rc = 130;
                            errorMessage = QStringLiteral( "-E-> During symbolizerName[%1.%2] no 'Fill' and 'Stroke' Elements in 'Symbolizer-Layers' were found.  ; rc=%3" ).arg( rendererType ).arg( symbolizerName ).arg( i_rc );
                          }
                          else
                          {
                            if ( fillElement.isNull() )
                            {
                              i_rc = 131;
                              errorMessage = QStringLiteral( "-E-> During symbolizerName[%1.%2] no 'Fill' Element in 'Symbolizer-Layers' was found.  ; rc=%3" ).arg( rendererType ).arg( symbolizerName ).arg( i_rc );
                            }
                            else if ( strokeElement.isNull() )
                            {
                              i_rc = 132;
                              errorMessage = QStringLiteral( "-E-> During symbolizerName[%1.%2] no 'Stroke' Element in 'Symbolizer-Layers' was found.  ; rc=%3" ).arg( rendererType ).arg( symbolizerName ).arg( i_rc );
                            }
                          }
                        }
                        if ( i_rc == 0 )
                        {
                          saLayers.append( symbolizerName );
                          errorMessage = QStringLiteral( "-I-> symbolizerName[%1.%2] all Elements in 'Symbolizer-Layers' were found.  ; rc=%3" ).arg( rendererType ).arg( symbolizerName ).arg( i_rc );
                        }
                      }
                    }
                    ruleChildElement = ruleChildElement.nextSiblingElement();
                  }
                  if ( saLayers.size() < 1 )
                  {
                    if ( i_rc == 0 )
                    {
                      i_rc = 108;
                      errorMessage = QStringLiteral( "-E-> [singleSymbol] 'Symbolizer-Layers' in [%1] were expected. ; rc=%2" ).arg( featTypeStyleElement.tagName() ).arg( i_rc );
                    }
                  }
                }
                else
                {
                  i_rc = 107;
                  testFirstElement = ruleChildElement.firstChildElement();
                  errorMessage = QStringLiteral( "-E-> 'Name' was expected from[%1] ; rc=%2 ; firstChildElement[%3]" ).arg( ruleChildElement.tagName() ).arg( i_rc ).arg( testFirstElement.tagName() );
                }
              }
              else
              {
                i_rc = 106;
                testFirstElement = featTypeStyleElement.firstChildElement();
                errorMessage = QStringLiteral( "-E-> 'Rule' was expected from[%1] ; rc=%2 ; firstChildElement[%3]" ).arg( featTypeStyleElement.tagName() ).arg( i_rc ).arg( testFirstElement.tagName() );
              }
            }
            else
            {
              if ( rendererType == "RuleRenderer" )
              {
                // Checking for the existence of a rule was done above [i_rc = 4]
                QDomElement ruleElement = featTypeStyleElement.firstChildElement( QStringLiteral( "Rule" ) );
                QDomElement nameRuleElement = ruleElement.firstChildElement( QStringLiteral( "Name" ) );
                QDomElement ruleChildElement = ruleElement.firstChildElement();
                QDomElement nameRuleChildElement = ruleChildElement.firstChildElement( QStringLiteral( "Name" ) );
                while ( !ruleChildElement.isNull() )
                {
                  // retrieve the Rule element child nodes [QgsRuleBasedRenderer::Rule::createFromSld]
                  QDomElement ruleGrandChildElement = ruleChildElement.firstChildElement();
                  sNodeName = ruleGrandChildElement.localName();
                  if ( sNodeName.isEmpty() )
                  {
                    // Note: an external source may not be using the 'se:' prefix, thus localName() will be empty.
                    sNodeName = ruleGrandChildElement.tagName();
                  }
                  while ( !ruleGrandChildElement.isNull() )
                  {
                    if ( ( sNodeName == QLatin1String( "Filter" ) ) ||
                         ( sNodeName == QLatin1String( "MinScaleDenominator" ) ) ||
                         ( sNodeName == QLatin1String( "MaxScaleDenominator" ) ) ||
                         ( sNodeName == QLatin1String( "Symbolizer" ) ) )
                    {
                      if ( sNodeName.endsWith( QLatin1String( "Symbolizer" ) ) )
                      {
                        // create symbol layers for this symbolizer
                        // QgsSymbolLayerUtils::createSymbolLayerListFromSld( childElem, geomType, layers );
                        QString symbolizerName = ruleGrandChildElement.nextSiblingElement().localName();
                        if ( sNodeName.isEmpty() )
                        {
                          // Note: an external source may not be using the 'se:' prefix, thus localName() will be empty.
                          symbolizerName = ruleGrandChildElement.nextSiblingElement().tagName();
                        }
                        if ( symbolizerName == QLatin1String( "PointSymbolizer" ) )
                        {
                          // first check for Graphic element, nothing will be rendered if not found
                          QDomElement graphicElement = ruleGrandChildElement.firstChildElement( QStringLiteral( "Graphic" ) );
                          if ( graphicElement.isNull() )
                          {
                            i_rc = 210;
                            errorMessage = QStringLiteral( "-E-> During symbolizerName[%1.%2] no 'Graphic' Element in 'Symbolizer-Layers' was found.  ; rc=%3" ).arg( rendererType ).arg( symbolizerName ).arg( i_rc );
                          }
                          else
                          {
                            saLayers.append( symbolizerName );
                            errorMessage = QStringLiteral( "-I-> symbolizerName[%1.%2] all Elements in 'Symbolizer-Layers' were found.  ; rc=%3" ).arg( rendererType ).arg( symbolizerName ).arg( i_rc );
                          }
                        }
                        else if ( symbolizerName == QLatin1String( "LineSymbolizer" ) )
                        {
                          // first check for Graphic element, nothing will be rendered if not found
                          QDomElement strokeElement = ruleGrandChildElement.firstChildElement( QStringLiteral( "Stroke" ) );
                          if ( strokeElement.isNull() )
                          {
                            i_rc = 220;
                            errorMessage = QStringLiteral( "-E-> During symbolizerName[%1.%2] no 'Stroke' Element in 'Symbolizer-Layers' was found.  ; rc=%3" ).arg( rendererType ).arg( symbolizerName ).arg( i_rc );
                          }
                          else
                          {
                            saLayers.append( symbolizerName );
                            errorMessage = QStringLiteral( "-I-> symbolizerName[%1.%2] all Elements in 'Symbolizer-Layers' were found.  ; rc=%3" ).arg( rendererType ).arg( symbolizerName ).arg( i_rc );
                          }
                        }
                        else if ( symbolizerName == QLatin1String( "PolygonSymbolizer" ) )
                        {
                          // get Fill and Stroke elements, nothing will be rendered if both are missing
                          QDomElement fillElement = ruleGrandChildElement.firstChildElement( QStringLiteral( "Fill" ) );
                          QDomElement strokeElement = ruleGrandChildElement.firstChildElement( QStringLiteral( "Stroke" ) );
                          if ( fillElement.isNull() || strokeElement.isNull() )
                          {
                            if ( fillElement.isNull() && strokeElement.isNull() )
                            {
                              i_rc = 230;
                              errorMessage = QStringLiteral( "-E-> During symbolizerName[%1.%2] no 'Fill' and 'Stroke' Elements in 'Symbolizer-Layers' were found.  ; rc=%3" ).arg( rendererType ).arg( symbolizerName ).arg( i_rc );
                            }
                            else
                            {
                              if ( fillElement.isNull() )
                              {
                                i_rc = 231;
                                errorMessage = QStringLiteral( "-E-> During symbolizerName[%1.%2] no 'Fill' Element in 'Symbolizer-Layers' was found.  ; rc=%3" ).arg( rendererType ).arg( symbolizerName ).arg( i_rc );
                              }
                              else if ( strokeElement.isNull() )
                              {
                                i_rc = 232;
                                errorMessage = QStringLiteral( "-E-> During symbolizerName[%1.%2] no 'Stroke' Element in 'Symbolizer-Layers' was found.  ; rc=%3" ).arg( rendererType ).arg( symbolizerName ).arg( i_rc );
                              }
                            }
                          }
                          else
                          {
                            saLayers.append( symbolizerName );
                            errorMessage = QStringLiteral( "-I-> symbolizerName[%1.%2] all Elements in 'Symbolizer-Layers' were found.  ; rc=%3" ).arg( rendererType ).arg( symbolizerName ).arg( i_rc );
                          }
                        }
                      }
                    }
                    ruleGrandChildElement = ruleGrandChildElement.nextSiblingElement();
                  }
                  ruleChildElement = ruleChildElement.nextSiblingElement( QStringLiteral( "Rule" ) );
                }
              }
              else
              {
                i_rc = 6;
                errorMessage = QStringLiteral( "-E-> Unexpected Renderer-Type[%1] was found ; rc=%2" ).arg( rendererType ).arg( i_rc );
              }
            }
          }
          else
          {
            i_rc = 5;
            testFirstElement = namedLayerElement.firstChildElement();
            errorMessage = QStringLiteral( "-E-> 'Rule' was expected from[%1] ; rc=%2 ; firstChildElement[%3]" ).arg( featTypeStyleElement.tagName() ).arg( i_rc ).arg( testFirstElement.tagName() );
          }
        }
        else
        {
          i_rc = 4;
          testFirstElement = namedLayerElement.firstChildElement();
          errorMessage = QStringLiteral( "-E-> 'FeatureTypeStyle' was expected from[%1] ; rc=%2 ; firstChildElement[%3]" ).arg( userStyleElement.tagName() ).arg( i_rc ).arg( testFirstElement.tagName() );
        }
      }
      else
      {
        i_rc = 3;
        testFirstElement = namedLayerElement.firstChildElement();
        errorMessage = QStringLiteral( "-E-> 'UserStyle' was expected from[%1] ; rc=%2 ; firstChildElement[%3]" ).arg( namedLayerElement.tagName() ).arg( i_rc ).arg( testFirstElement.tagName() );
      }
    }
    else
    {
      i_rc = 2;
      testFirstElement = namedLayerElement.firstChildElement();
      errorMessage = QStringLiteral( "-E-> 'Name' was expected from[%1]; rc=%2 ; firstChildElement[%3]" ).arg( namedLayerElement.tagName() ).arg( i_rc ).arg( testFirstElement.tagName() );
    }
  }
  else
  {
    i_rc = 1;
    errorMessage = QString( "--E---> SpatialiteDbInfo::testLayerStyleNamedLayerElement: NamedLayer[%1]: not found ; rc=%2" ).arg( namedLayerElement.tagName() ).arg( i_rc );
  }
  return i_rc;
}
//-----------------------------------------------------------------
// Class SpatialiteDbLayer
//-----------------------------------------------------------------
// SpatialiteDbLayer::~SpatialiteDbLayer
//-----------------------------------------------------------------
SpatialiteDbLayer::SpatialiteDbLayer( SpatialiteDbInfo *dbConnectionInfo )
  : mDbSpatialiteInfo( dbConnectionInfo )
  , mDatabaseFileName( dbConnectionInfo->getDatabaseFileName() )
  , mTableName( QString::null )
  , mGeometryColumn( QString::null )
  , mLayerName( QString::null )
  , mListGroupName( QString::null )
  , mCoverageName( QString::null )
  , mViewTableName( QString::null )
  , mViewTableGeometryColumn( QString::null )
  , mLayerViewTableName( QString::null )
  , mTitle( QString::null )
  , mAbstract( QString::null )
  , mCopyright( QString::null )
  , mLicense( QString::null )
  , mSrid( -2 )
  , mAuthId( QString::null )
  , mProj4text( QString::null )
  , mSpatialIndexType( SpatialiteDbInfo::SpatialIndexNone )
  , mLayerType( SpatialiteDbInfo::SpatialiteUnknown )
  , mLayerTypeString( QString::null )
  , mLayerInfo( QString::null )
  , mDataSourceUri( QString::null )
  , mIsSpatialite( false )
  , mIsRasterLite2( false )
  , mGeometryType( QgsWkbTypes::Unknown )
  , mGeometryTypeString( QString::null )
  , mCoordDimensions( GAIA_XY )
  , mLayerExtent( QgsRectangle( 0.0, 0.0, 0.0, 0.0 ) )
  , mLayerResolution( QgsPointXY( 0.0, 0.0 ) )
  , mLayerImageExtent( QPoint( 0, 0 ) )
  , mLayerBandsTileSize( QRect( 0, 0, 0, 0 ) )
  , mLayerSampleType( QString::null )
  , mLayerRasterDataType( Qgis::UnknownDataType )
  , mLayerPixelType( 16 )
  , mLayerPixelTypeString( QString::null )
  , mLayerCompressionType( QString::null )
  , mLayerExtentEWKT( QString::null )
  , mLayerExtentWsg84( QgsRectangle( 0.0, 0.0, 0.0, 0.0 ) )
  , mLayerExtentWsg84EWKT( QString::null )
  , mLayerStyleSelected( -1 )
  , mLayerStyleSelectedName( QString::null )
  , mLayerStyleSelectedTitle( QString::null )
  , mLayerStyleSelectedAbstract( QString::null )
  , mNumberFeatures( 0 )
  , mLayerReadOnly( 1 )
  , mLayerIsHidden( 0 )
  , mTriggerInsert( false )
  , mTriggerUpdate( false )
  , mTriggerDelete( false )
  , mLayerId( -1 )
  , mEnabledCapabilities( QgsVectorDataProvider::NoCapabilities )
  , mQuery( QString::null )
  , mPrimaryKey( QString::null )
  , mPrimaryKeyCId( -1 )
  , mIsLayerValid( false )
  , mHasStyle( false )
  , mStyleType( SpatialiteDbInfo::SpatialiteUnknown )
{
  mAttributeFields.clear();
  mPrimaryKey.clear(); // cazzo cazzo cazzo
  mPrimaryKeyAttrs.clear();
  mDefaultValues.clear();
  mTopologyExportLayers.clear();
  mTopologyExportLayersDataSourceUris.clear();
};
//-----------------------------------------------------------------
// SpatialiteDbLayer::~SpatialiteDbLayer
//-----------------------------------------------------------------
SpatialiteDbLayer::~SpatialiteDbLayer()
{
  for ( QMap<QString, SpatialiteDbLayer *>::iterator it = mTopologyExportLayers.begin(); it != mTopologyExportLayers.end(); ++it )
  {
    SpatialiteDbLayer *dbLayer = qobject_cast<SpatialiteDbLayer *>( it.value() );
    delete dbLayer;
  }
  mTopologyExportLayers.clear();
  mDefaultValues.clear();
  mErrors.clear();
  mWarnings.clear();
  mLayerStyleInfo.clear();
  mTopologyExportLayersDataSourceUris.clear();
  mDbSpatialiteInfo = nullptr; // do not delete
};
//-----------------------------------------------------------------
// SpatialiteDbLayer::layerHasRasterlite2
//-----------------------------------------------------------------
bool SpatialiteDbLayer::layerHasRasterlite2()
{
  bool bRc = false;
  if ( getDbSpatialiteInfo() )
  {
    if ( ( getLayerType() == SpatialiteDbInfo::RasterLite2Raster ) || ( getLayerType() == SpatialiteDbInfo::RasterLite2Vector ) )
    {
      if ( getDbSpatialiteInfo()->dbRasterLite2VersionMajor() < 0 )
      {
        getDbSpatialiteInfo()->dbHasRasterlite2();
      }
      if ( getDbSpatialiteInfo()->dbRasterLite2VersionMajor() > 0 )
      {
        bRc = true;
      }
    }
  }
  return bRc;
}
bool SpatialiteDbInfo::UpdateLayerStatistics( QStringList saLayers )
{
  bool bRc = false;
  if ( ( isDbValid() ) && ( isDbSpatialite() ) )
  {
    char *errMsg = nullptr;
    if ( saLayers.size() < 1 )
    {
      // UpdateLayerStatistics for whole Database
      saLayers << QString();
    }
    if ( sqlite3_exec( dbSqliteHandle(), "BEGIN", nullptr, nullptr, &errMsg ) == SQLITE_OK )
    {
      bRc = true;
      for ( int i = 0; i < saLayers.size(); i++ )
      {
        QString sql;
        QString sLayerName = saLayers.at( i ).trimmed();
        QString sTableName = sLayerName;
        QString sGeometryColumn = QString();
        // Since this can be called from an external class, check if the Database file name was given
        if ( ( !sTableName.isEmpty() ) && ( sTableName.contains( getFileName(), Qt::CaseInsensitive ) ) )
        {
          // Assume that the whole Database is being requested [should never happen, but one never knows ...]
          sTableName = QString();
        }
        if ( !sTableName.isEmpty() )
        {
          //----------------------------------------------------------
          // Extract TableName/GeometryColumn from sent 'table_name' or 'table_name(field_name)' from LayerName
          SpatialiteDbInfo::parseLayerName( sLayerName, sTableName, sGeometryColumn );
          //----------------------------------------------------------
          if ( mIsSpatialite40 )
          {
            sql = QStringLiteral( "SELECT InvalidateLayerStatistics('%1'%2),UpdateLayerStatistics('%1'%2)" ).arg( sTableName ).arg( sGeometryColumn );
          }
          else
          {
            sql = QStringLiteral( "SELECT UpdateLayerStatistics('%1'%2)" ).arg( sTableName ).arg( sGeometryColumn );
          }
        }
        else
        {
          if ( mIsSpatialite40 )
          {
            sql = QStringLiteral( "SELECT InvalidateLayerStatistics(),UpdateLayerStatistics()" );
          }
          else
          {
            sql = QStringLiteral( "SELECT UpdateLayerStatistics()" );
          }
        }
        if ( sqlite3_exec( dbSqliteHandle(), sql.toUtf8().constData(), nullptr, nullptr, &errMsg ) != SQLITE_OK )
        {
          bRc = false;
          QgsMessageLog::logMessage( QObject::tr( "SQLite error: %2\nSQL: %1" ).arg( sql, errMsg ? errMsg : "" ), QObject::tr( "SpatiaLite" ) );
          continue;
        }
        if ( sTableName.isEmpty() )
        {
          // ignore any other commands  since the whole Database has been done [should never happen]
          continue;
        }
      }
      if ( sqlite3_exec( dbSqliteHandle(), "COMMIT", nullptr, nullptr, &errMsg ) != SQLITE_OK )
      {
        bRc = false;
      }
    }
  }
  return bRc;
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::UpdateLayerStatistics
//-----------------------------------------------------------------
bool SpatialiteDbLayer::UpdateLayerStatistics()
{
  if ( ( isLayerValid() ) && ( getDbSpatialiteInfo()->isDbSpatialite() ) )
  {
    return getDbSpatialiteInfo()->UpdateLayerStatistics( ( QStringList() << mLayerName ) );
  }
  return false;
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::setLayerExtent
//-----------------------------------------------------------------
QString SpatialiteDbLayer::setLayerExtent( QgsRectangle layerExtent, QgsPointXY layerResolution )
{
  mLayerExtent = layerExtent;
  // Default value of zMin/Max is 0.0
  mLayerExtent3d = QgsBox3d();
  mLayerExtent3d.setXMinimum( mLayerExtent.xMinimum() );
  mLayerExtent3d.setXMaximum( mLayerExtent.xMaximum() );
  mLayerExtent3d.setYMinimum( mLayerExtent.yMinimum() );
  mLayerExtent3d.setYMaximum( mLayerExtent.yMaximum() );
  mLayerResolution = layerResolution;
  mLayerExtentEWKT = QString( "'SRID=%1;%2'" ).arg( getSrid() ).arg( mLayerExtent.asWktPolygon() );
  if ( ( mLayerResolution.x() != 0.0 ) && ( mLayerResolution.y() != 0.0 ) )
  {
    mLayerImageExtent.setX( ( int )( mLayerExtent.width() / getLayerImageResolutionX() ) );
    mLayerImageExtent.setY( ( int )( mLayerExtent.height() / getLayerImageResolutionY() ) );
  }
  return mLayerExtentEWKT;
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::setLayerExtentWsg84
//-----------------------------------------------------------------
QString SpatialiteDbLayer::setLayerExtentWsg84( QgsRectangle layerExtent )
{
  mLayerExtentWsg84 = layerExtent;
  mLayerExtentWsg84EWKT = QString( "'SRID=%1;%2'" ).arg( 4326 ).arg( mLayerExtent.asWktPolygon() );
  // Default value of zMin/Max is 0.0
  mLayerExtent3dWsg84.setXMinimum( mLayerExtentWsg84.xMinimum() );
  mLayerExtent3dWsg84.setXMaximum( mLayerExtentWsg84.xMaximum() );
  mLayerExtent3dWsg84.setYMinimum( mLayerExtentWsg84.yMinimum() );
  mLayerExtent3dWsg84.setYMaximum( mLayerExtentWsg84.yMaximum() );
  return mLayerExtentWsg84EWKT;
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::setLayerExtents
//-----------------------------------------------------------------
int SpatialiteDbLayer::setLayerExtents( QList<QString> layerExtents )
{
  mLayerExtents.clear();
  for ( int i = 0; i < layerExtents.size(); i++ )
  {
    QStringList sa_extent = layerExtents.at( i ).split( SpatialiteDbInfo::ParseSeparatorCoverage );
    if ( sa_extent.size() == 10 )
    {
      // Value: native_srid€srid€;extent_minx€extent_miny€extent_maxx€extent_maxy€geo_minx€geo_miny€geo_maxx€geo_maxy
      int i_native_srid = sa_extent.at( 0 ).toInt();
      int i_srid = sa_extent.at( 1 ).toInt();
      double dExtentMinX = sa_extent.at( 2 ).toDouble();
      double dExtentMinY = sa_extent.at( 3 ).toDouble();
      double dExtentMaxX = sa_extent.at( 4 ).toDouble();
      double dExtentMaxY = sa_extent.at( 5 ).toDouble();
      double dGeoMinX = sa_extent.at( 6 ).toDouble();
      double dGeoMinY = sa_extent.at( 7 ).toDouble();
      double dGeoMaxX = sa_extent.at( 8 ).toDouble();
      double dGeoMaxY = sa_extent.at( 9 ).toDouble();
      if ( i_native_srid == 1 )
      {
        if ( mSrid != i_srid )
        {
          // do only if not set allready, will also set mAuthId and mProj4text
          setSrid( i_srid );
        }
        // Will also set mLayerExtent3d and mLayerExtentEWKT
        setLayerExtent( QgsRectangle( dExtentMinX, dExtentMinY, dExtentMaxX, dExtentMaxY ) );
        mLayerExtents.insert( mSrid, mLayerExtent3d );
        // Will also set mLayerExtent3dWsg84 and mLayerExtentWsg84EWKT
        setLayerExtentWsg84( QgsRectangle( dGeoMinX, dGeoMinY, dGeoMaxX, dGeoMaxY ) );
        mLayerExtents.insert( 4326, mLayerExtent3dWsg84 );
      }
      else
      {
        // do not add Wsg84 again
        if ( i_srid != 4326 )
        {
          QgsBox3d addBox3d = QgsBox3d();
          addBox3d.setXMinimum( dExtentMinX );
          addBox3d.setXMaximum( dExtentMinY );
          addBox3d.setYMinimum( dExtentMaxX );
          addBox3d.setYMaximum( dExtentMaxY );
          mLayerExtents.insert( i_srid, addBox3d );
        }
      }
    }
  }
  return mLayerExtents.count();
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::prepare
//-----------------------------------------------------------------
bool SpatialiteDbLayer::prepare()
{
  //---------------------------------------------------------------
  switch ( mLayerType )
  {
    case SpatialiteDbInfo::SpatialView:
    {
      if ( getTableName().contains( "_" ) )
      {
        QStringList sa_list_name = getTableName().toLower().split( "_" );
        if ( sa_list_name.size() >= 2 )
        {
          mListGroupName = QString( "%1_%2" ).arg( sa_list_name.at( 0 ) ).arg( sa_list_name.at( 1 ) );
        }
      }
    }
    break;
    default:
      break;
  }
  //---------------------------------------------------------------
  mCoverageName = mLayerName.toLower();
  //---------------------------------------------------------------
  QString sInfoType = getLayerTypeString();
  switch ( mLayerType )
  {
    case SpatialiteDbInfo::SpatialTable:
    case SpatialiteDbInfo::SpatialView:
    case SpatialiteDbInfo::VirtualShape:
    case SpatialiteDbInfo::GdalFdoOgr:
    case SpatialiteDbInfo::GeoPackageVector:
      sInfoType = getGeometryTypeString();
      break;
    default:
      break;
  }
  //---------------------------------------------------------------
  getDbSpatialiteInfo()->createDbLayerInfoUri( mLayerInfo, mDataSourceUri, mLayerName, mLayerType, sInfoType, mSrid, ( int )mSpatialIndexType, mLayerIsHidden );
  //---------------------------------------------------------------
  // 'createDbLayerInfoUri' must run before 'checkLayerStyles'
  //---------------------------------------------------------------
  checkLayerStyles();
  //---------------------------------------------------------------
  // 'checkLayerStyles' must run before 'setLayerExtents'
  //---------------------------------------------------------------
  if ( getDbSpatialiteInfo()->getDbVectorCoveragesLayersExtent().contains( mLayerName ) )
  {
    // Retrieve all entries for this Vector-Layer (if any)
    setLayerExtents( getDbSpatialiteInfo()->getDbVectorCoveragesLayersExtent().values( mLayerName ) );
  }
  if ( getDbSpatialiteInfo()->getDbRasterCoveragesLayersExtent().contains( mLayerName ) )
  {
    // Retrieve all entries for this Raster-Layer (if any)
    setLayerExtents( getDbSpatialiteInfo()->getDbRasterCoveragesLayersExtent().values( mLayerName ) );
  }
  //---------------------------------------------------------------
  if ( mLayerExtents.count() ==  0 )
  {
    // There are no VectorCoverages or RasterCoverages
    mLayerExtents.insert( mSrid, mLayerExtent3d );
    if ( mLayerExtentWsg84EWKT.size() > 0 )
    {
      mLayerExtents.insert( 4326, mLayerExtent3dWsg84 );
    }
  }
  //---------------------------------------------------------------
  if ( mEnabledCapabilities == QgsVectorDataProvider::NoCapabilities )
  {
    getCapabilities();
  }
  //---------------------------------------------------------------
  // QgsLayerMetadata for Layer
  //---------------------------------------------------------------
  setLayerMetadata();
  //---------------------------------------------------------------
  return isLayerValid();
};
//-----------------------------------------------------------------
// SpatialiteDbLayer::setLayerMetadata
//-----------------------------------------------------------------
bool SpatialiteDbLayer::setLayerMetadata()
{
  //---------------------------------------------------------------
  // QgsLayerMetadata for Layer
  //---------------------------------------------------------------
  mLayerMetadata.setIdentifier( getLayerDataSourceUri() );
  mLayerMetadata.setParentIdentifier( getDatabaseUri() );
  mLayerMetadata.setCrs( QgsCoordinateReferenceSystem::fromOgcWmsCrs( getSridEpsg() ) );
  mLayerMetadata.setTitle( getTitle() );
  mLayerMetadata.setAbstract( getAbstract() );
  mLayerMetadata.setRights( QStringList( getCopyright() ) );
  mLayerMetadata.setLicenses( QStringList( getLicense() ) );
  mLayerMetadata.setEncoding( QStringLiteral( "UTF-8" ) );
// Extent of Layer in set Srid
  QgsLayerMetadata::Extent metadataExtent;
  QList< QgsLayerMetadata::SpatialExtent > metadataSpatialExtents;
  QgsLayerMetadata::SpatialExtent se = QgsLayerMetadata::SpatialExtent();
  se.extentCrs = mLayerMetadata.crs();
  se.bounds = mLayerExtent3d;
  metadataSpatialExtents.append( se );
  if ( mLayerExtentWsg84EWKT.size() > 0 )
  {
    // If set: Extent as Wsg84
    se = QgsLayerMetadata::SpatialExtent();
    se.extentCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( QString( "EPSG:%1" ).arg( 4326 ) );
    se.bounds = mLayerExtent3dWsg84;
    metadataSpatialExtents.append( se );
  }
  if ( mLayerExtents.count() > 0 )
  {
    for ( QMap<int, QgsBox3d>::iterator it = mLayerExtents.begin(); it != mLayerExtents.end(); ++it )
    {
      int i_srid = it.key();
      if ( ( i_srid != mSrid ) && ( i_srid != 4326 ) )
      {
        // Add only what has not already been added
        se = QgsLayerMetadata::SpatialExtent();
        se.extentCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( QString( "EPSG:%1" ).arg( i_srid ) );
        se.bounds = it.value();
        metadataSpatialExtents.append( se );
      }
    }
  }
  metadataExtent.setSpatialExtents( metadataSpatialExtents );
  mLayerMetadata.setExtent( metadataExtent );
  mLayerMetadata.setType( getLayerTypeString() );
  // mLayerMetadata.setConstraints()
  // setHistory()
  // setKeywords(
  // setCategories(
  // addContact(
  // setLinks(
  //---------------------------------------------------------------
  return isLayerValid();
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::setLayerInvalid
//-----------------------------------------------------------------
int SpatialiteDbLayer::setLayerInvalid( QString sLayerName, QString errCause )
{
  mIsLayerValid = false;
  if ( !errCause.isEmpty() )
  {
    mErrors.insert( sLayerName, errCause );
  }
  return errCause.size();
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::setSpatialIndexType
//-----------------------------------------------------------------
void SpatialiteDbLayer::setSpatialIndexType( int iSpatialIndexType )
{
  mSpatialIndexType = ( SpatialiteDbInfo::SpatialIndexType )iSpatialIndexType;
  mSpatialIndexTypeString = SpatialiteDbInfo::SpatialIndexTypeName( mSpatialIndexType );
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::setLayerType
//-----------------------------------------------------------------
void SpatialiteDbLayer::setLayerType( SpatialiteDbInfo::SpatialiteLayerType layerType )
{
  mLayerType = layerType;
  mLayerTypeString = SpatialiteDbInfo::SpatialiteLayerTypeName( mLayerType );
  switch ( mLayerType )
  {
    case SpatialiteDbInfo::SpatialTable:
    case SpatialiteDbInfo::SpatialView:
    case SpatialiteDbInfo::VirtualShape:
    case SpatialiteDbInfo::RasterLite2Raster:
    case SpatialiteDbInfo::TopologyExport:
      mIsSpatialite = true;
      if ( mLayerType == SpatialiteDbInfo::RasterLite2Raster )
      {
        mIsRasterLite2 = true;
      }
      break;
    case SpatialiteDbInfo::AllSpatialLayers:
    case SpatialiteDbInfo::GdalFdoOgr:
    case SpatialiteDbInfo::GeoPackageVector:
    case SpatialiteDbInfo::RasterLite1:
    case SpatialiteDbInfo::GeoPackageRaster:
    case SpatialiteDbInfo::MBTilesTable:
    case SpatialiteDbInfo::MBTilesView:
    case SpatialiteDbInfo::SpatialiteTopology:
    case SpatialiteDbInfo::StyleVector:
    case SpatialiteDbInfo::StyleRaster:
    case SpatialiteDbInfo::NonSpatialTables:
    case SpatialiteDbInfo::AllLayers:
    default:
      mIsSpatialite = false;
      break;
  }
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::setGeometryType
//-----------------------------------------------------------------
void SpatialiteDbLayer::setGeometryType( int iGeomType )
{
  mGeometryType = ( QgsWkbTypes::Type )iGeomType;
  mGeometryTypeString = QgsWkbTypes::displayString( mGeometryType );
  if ( mGeometryTypeString.endsWith( QLatin1String( "ZM" ) ) )
  {
    mCoordDimensions = GAIA_XY_Z_M;
  }
  else if ( mGeometryTypeString.endsWith( QLatin1String( "M" ) ) )
  {
    mCoordDimensions = GAIA_XY_M;
  }
  else if ( mGeometryTypeString.endsWith( QLatin1String( "Z" ) ) )
  {
    mCoordDimensions = GAIA_XY_Z_M;
  }
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::getLayerStyleXml
//-----------------------------------------------------------------
QString SpatialiteDbLayer::getLayerStyleXml( int iStyleId )
{
  if ( !mLayerStyleInfo.isEmpty() )
  {
    if ( iStyleId < 0 )
    {
      iStyleId = mLayerStyleSelected;
    }
    switch ( mStyleType )
    {
      case SpatialiteDbInfo::StyleVector:
      {
        return getDbSpatialiteInfo()->getDbVectorStyleXml( iStyleId );
      }
      break;
      case SpatialiteDbInfo::StyleRaster:
      {
        return getDbSpatialiteInfo()->getDbRasterStyleXml( iStyleId );
      }
      break;
      default:
        break;
    }
  }
  return QString();
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::getLayerStyleNamedLayerElement
//-----------------------------------------------------------------
// - retrieve xml of style_id for rendering on layer
// -- if not set,  'selected' value will be used
// -- otherwise first style will be used (if any)
// - checking is done that the style_id exists
//-----------------------------------------------------------------
QDomElement SpatialiteDbLayer::getLayerStyleNamedLayerElement( int iStyleId, QString errorMessage )
{
  if ( iStyleId < 0 )
  {
    // if called without value [-1]
    if ( mLayerStyleSelected >= 0 )
    {
      // if selected is set use selected value
      iStyleId = mLayerStyleSelected;
    }
    if ( ( iStyleId < 0 ) && ( mStylesId.size() ) )
    {
      // if still not set, used first value [default]
      iStyleId = mStylesId.at( 0 );
    }
  }
  // request only if value is contained in array of existing style_id's
  if ( mStylesId.contains( iStyleId ) )
  {
    return getDbSpatialiteInfo()->getDbStyleNamedLayerElement( mStyleType, iStyleId, errorMessage, nullptr, QString::null );
  }
  return QDomElement();
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::checkLayerStyles
//-----------------------------------------------------------------
bool SpatialiteDbLayer::checkLayerStyles()
{
  //---------------------------------------------------------------
  if ( ( isLayerValid() ) && ( getDbSpatialiteInfo()->isDbSpatialite() ) )
  {
    QString sql;
    QString sLayerType = getLayerTypeString();
    QString sStyleType;
    QString sCoverageData;
    mLayerStyleInfo.clear();
    switch ( mLayerType )
    {
      case SpatialiteDbInfo::SpatialTable:
      case SpatialiteDbInfo::SpatialView:
      case SpatialiteDbInfo::VirtualShape:
      case SpatialiteDbInfo::TopologyExport:
      {
        mStyleType = SpatialiteDbInfo::StyleVector;
        if ( getDbSpatialiteInfo()->dbVectorCoveragesLayersCount() > 0 )
        {
          sCoverageData = getDbSpatialiteInfo()->getDbVectorCoveragesLayers().value( mLayerName );
          sLayerType = SpatialiteDbInfo::SpatialiteLayerTypeName( SpatialiteDbInfo::RasterLite2Vector );
          QString sCheckLayerType;
          // Use the Euro-Character to parse, hoping it will not be used in the Title/Abstract Text
          if ( getDbSpatialiteInfo()->parseLayerCoverage( sCoverageData, sCheckLayerType, mCoverageName, mTitle, mAbstract, mCopyright, mLicense, mSrid ) )
          {
            if ( sCheckLayerType == sLayerType )
            {
              // Valid
            }
          }
        }
        // No Coverage-Name, no Style
        if ( ( !mCoverageName.isEmpty() ) && ( getDbSpatialiteInfo()->dbVectorStylesViewsCount() > 0 ) )
        {
          mStylesId = getDbSpatialiteInfo()->getLayerNamesStyles().values( mCoverageName );
          // Retrieve only those Styles being used  (ignore the others)
          // sql = QString( "SELECT name FROM SE_vector_styled_layers_view WHERE (coverage_name='%1')" ).arg( mCoverageName );
          // i_rc = SQLITE_OK;
        }
      }
      break;
      case SpatialiteDbInfo::RasterLite2Raster:
      {
        mCoverageName = mLayerName.toLower();
        mStyleType = SpatialiteDbInfo::StyleRaster;
        sLayerType = SpatialiteDbInfo::SpatialiteLayerTypeName( SpatialiteDbInfo::RasterLite2Raster );
        // Here the coverage name is the same as our LayerName [Title,Abstract and Copyright have been retrieved elsewhere]
        if ( getDbSpatialiteInfo()->dbRasterStylesViewCount() > 0 )
        {
          QString sCoverageData = getDbSpatialiteInfo()->getDbRasterCoveragesLayers().value( mLayerName );
          // Use the Euro-Character to parse, hoping it will not be used in the Title/Abstract Text
          QString sCheckLayerType;
          if ( getDbSpatialiteInfo()->parseLayerCoverage( sCoverageData, sCheckLayerType, mCoverageName, mTitle, mAbstract, mCopyright, mLicense, mSrid ) )
          {
            if ( sCheckLayerType == sLayerType )
            {
              // Valid
            }
          }
          mStylesId = getDbSpatialiteInfo()->getLayerNamesStyles().values( mCoverageName );
          // sql = QString( "SELECT name FROM SE_raster_styled_layers_view WHERE (coverage_name='%1')" ).arg( mCoverageName );
          // i_rc = SQLITE_OK;
        }
      }
      break;
      default:
        break;
    }
    if ( mStylesId.size() > 0 )
    {
      mLayerStyleSelected = mStylesId.at( 0 );
      mHasStyle = true;
    }
  }
  //---------------------------------------------------------------
  return isLayerValid();
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::setLayerStyleSelected
//-----------------------------------------------------------------
int SpatialiteDbLayer::setLayerStyleSelected( int iStyleId )
{
  mHasStyle = false;
  switch ( mStyleType )
  {
    case SpatialiteDbInfo::StyleVector:
    {
      if ( getDbSpatialiteInfo()->dbVectorStylesCount() > 0 )
      {
      }
    }
    break;
    case SpatialiteDbInfo::StyleRaster:
    {
      if ( getDbSpatialiteInfo()->dbRasterStylesCount() > 0 )
      {
      }
    }
    break;
    default:
      break;
  }
  if ( !mLayerStyleInfo.isEmpty() )
  {
    if ( iStyleId < 0l )
    {
      iStyleId = mLayerStyleInfo.firstKey();
    }
    if ( mLayerStyleInfo.contains( iStyleId ) )
    {
      QString sStyleInfo = mLayerStyleInfo.value( iStyleId );
      QStringList sa_style_info = sStyleInfo.split( SpatialiteDbInfo::ParseSeparatorCoverage );
      if ( sa_style_info.size() == 4 )
      {
        if ( sa_style_info.at( 3 ).startsWith( "valid" ) )
        {
          mLayerStyleSelected = iStyleId ;
          QgsDebugMsgLevel( QString( "-I-> SpatialiteDbLayer::setLayerStyleSelected: mLayerStyleSelected[%1] " ).arg( mLayerStyleSelected ), 4 );
          // sStyleXml = QString( "%2%1%3%1%4%1%5" ).arg( SpatialiteDbInfo::ParseSeparatorCoverage ).arg( sStyleName ).arg( sStyleTitle ).arg( sStyleAbstract ).arg( sTestStylesForQgis );
          mLayerStyleSelectedName = sa_style_info.at( 0 );
          mLayerStyleSelectedTitle = sa_style_info.at( 1 );
          mLayerStyleSelectedAbstract = sa_style_info.at( 2 );
          mHasStyle = true;
        }
      }
    }
    if ( ( mLayerType == SpatialiteDbInfo::RasterLite2Raster ) && ( mLayerStyleSelectedString.isEmpty() ) )
    {
      mLayerStyleSelectedString = QStringLiteral( "default" );
    }
  }
  return mLayerStyleSelected;
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::getCapabilities
//-----------------------------------------------------------------
QgsVectorDataProvider::Capabilities SpatialiteDbLayer::getCapabilities( bool bUpdate )
{
  //---------------------------------------------------------------
  if ( ( mEnabledCapabilities == QgsVectorDataProvider::NoCapabilities ) || ( bUpdate ) )
  {
    if ( bUpdate )
    {
      GetLayerSettings();
    }
    mEnabledCapabilities = mPrimaryKey.isEmpty() ? QgsVectorDataProvider::Capabilities() : ( QgsVectorDataProvider::SelectAtId );
    switch ( mLayerType )
    {
      case SpatialiteDbInfo::GdalFdoOgr:
      case SpatialiteDbInfo::TopologyExport:
      case SpatialiteDbInfo::SpatialTable:
        mEnabledCapabilities |= QgsVectorDataProvider::DeleteFeatures | QgsVectorDataProvider::FastTruncate;
        if ( !mGeometryColumn.isEmpty() )
          mEnabledCapabilities |= QgsVectorDataProvider::ChangeGeometries;
        mEnabledCapabilities |= QgsVectorDataProvider::ChangeAttributeValues;
        mEnabledCapabilities |= QgsVectorDataProvider::AddFeatures;
        mEnabledCapabilities |= QgsVectorDataProvider::AddAttributes;
        mEnabledCapabilities |= QgsVectorDataProvider::CreateAttributeIndex;
        break;
      case SpatialiteDbInfo::SpatialView:
        if ( !mLayerReadOnly )
        {
          if ( mTriggerDelete )
          {
            mEnabledCapabilities |= QgsVectorDataProvider::DeleteFeatures | QgsVectorDataProvider::FastTruncate;
          }
          if ( mTriggerUpdate )
          {
            mEnabledCapabilities |= QgsVectorDataProvider::ChangeGeometries;
            mEnabledCapabilities |= QgsVectorDataProvider::ChangeAttributeValues;
          }
          if ( mTriggerInsert )
          {
            mEnabledCapabilities |= QgsVectorDataProvider::AddFeatures;
            mEnabledCapabilities |= QgsVectorDataProvider::AddAttributes;
            mEnabledCapabilities |= QgsVectorDataProvider::CreateAttributeIndex;
          }
        }
        break;
      case SpatialiteDbInfo::GeoPackageVector:
      case SpatialiteDbInfo::GeoPackageRaster:
      case SpatialiteDbInfo::RasterLite1:
      case SpatialiteDbInfo::RasterLite2Raster:
      case SpatialiteDbInfo::MBTilesTable:
      case SpatialiteDbInfo::MBTilesView:
      case SpatialiteDbInfo::SpatialiteTopology:
      case SpatialiteDbInfo::StyleVector:
      case SpatialiteDbInfo::StyleRaster:
      case SpatialiteDbInfo::NonSpatialTables:
        mEnabledCapabilities = QgsVectorDataProvider::NoCapabilities;
        break;
      default:
        setLayerInvalid( mLayerName, QString( "[%1] Capabilities: Unexpected Layer-Type[%2]." ).arg( mLayerName ).arg( getLayerTypeString() ) );
        break;
    }
  }
  //---------------------------------------------------------------
  return mEnabledCapabilities;
};
//-----------------------------------------------------------------
// SpatialiteDbLayer::setSrid
//-----------------------------------------------------------------
bool SpatialiteDbLayer::setSrid( int iSrid )
{
  bool bRc = false;
  int i_rc = 0;
  if ( !getDbSpatialiteInfo()->isDbSpatialite() )
  {
    mSrid = iSrid;
    bRc = true;
    mIsLayerValid = bRc;
    return bRc;
  }
  sqlite3_stmt *stmt = nullptr;
  QString sql;
  sql = QStringLiteral( "SELECT auth_name||':'||auth_srid,proj4text FROM spatial_ref_sys WHERE srid=%1" ).arg( iSrid );
  i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
  if ( i_rc == SQLITE_OK )
  {
    while ( sqlite3_step( stmt ) == SQLITE_ROW )
    {
      if ( ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL ) &&
           ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL ) )
      {
        mAuthId = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
        mProj4text = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
        mSrid = iSrid;
        bRc = true;
      }
    }
    sqlite3_finalize( stmt );
  }
  mIsLayerValid = bRc;
  return isLayerValid();
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::getLayerExtent
//-----------------------------------------------------------------
QgsRectangle SpatialiteDbLayer::getLayerExtent( bool bUpdateExtent, bool bUpdateStatistics )
{
  // Note: When this is called from addLayerFeatures, saved new geometries cannot be selected
  // - the problem occurs irregularly, but persistently - so will not be called
  if ( ( isLayerValid() ) && ( ( bUpdateExtent ) || ( bUpdateStatistics ) ) )
  {
    int i_rc = 0;
    sqlite3_stmt *stmt = nullptr;
    QString sql;
    QString sourceTable;
    QString parmTable;
    QString parmField;
    QString parmGeometryColumn;
    QString sFields;
    switch ( mLayerType )
    {
      case SpatialiteDbInfo::TopologyExport:
      case SpatialiteDbInfo::SpatialTable:
      case SpatialiteDbInfo::SpatialView:
      case SpatialiteDbInfo::VirtualShape:
        sourceTable = "vector_layers_statistics";
        parmTable = "table_name";
        parmField = "geometry_column";
        parmGeometryColumn = QString( " AND (%1 = '%2')" ).arg( parmField ).arg( mGeometryColumn );
        sFields = QString( "extent_min_x,extent_min_y,extent_max_x,extent_max_y, row_count" );
        break;
      case SpatialiteDbInfo::RasterLite2Vector:
        sourceTable = "vector_coverages";
        parmTable = "coverage_name";
        parmField = "";
        parmGeometryColumn = "";
        sFields = QString( "extent_minx,extent_miny,extent_maxx,extent_maxy" );
        break;
      case SpatialiteDbInfo::RasterLite2Raster:
        sourceTable = "raster_coverages";
        parmTable = "coverage_name";
        parmField = "";
        parmGeometryColumn = "";
        sFields = QString( "extent_minx,extent_miny,extent_maxx,extent_maxy" );
        break;
      default:
        bUpdateExtent = false;
        bUpdateStatistics = false;
        break;
    }
    if ( bUpdateStatistics )
    {
      sql = QStringLiteral( "SELECT InvalidateLayerStatistics('%1','%2'),UpdateLayerStatistics('%1','%2')" ).arg( mTableName ).arg( mGeometryColumn );
      ( void )sqlite3_exec( dbSqliteHandle(), sql.toUtf8().constData(), nullptr, 0, nullptr );
    }
    if ( bUpdateExtent )
    {
      sql = QStringLiteral( "SELECT %1 FROM %2 WHERE (%3 = '%4')%5" ).arg( sFields ).arg( sourceTable ).arg( parmTable ).arg( mTableName ).arg( parmGeometryColumn );
      i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
      if ( i_rc == SQLITE_OK )
      {
        while ( sqlite3_step( stmt ) == SQLITE_ROW )
        {
          if ( ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL ) &&
               ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL ) &&
               ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL ) &&
               ( sqlite3_column_type( stmt, 3 ) != SQLITE_NULL ) )
          {
            QgsRectangle layerExtent( sqlite3_column_double( stmt, 0 ), sqlite3_column_double( stmt, 1 ), sqlite3_column_double( stmt, 2 ), sqlite3_column_double( stmt, 3 ) );
            setLayerExtent( layerExtent );
          }
          if ( sqlite3_column_type( stmt, 4 ) != SQLITE_NULL )
          {
            mNumberFeatures = sqlite3_column_int64( stmt, 4 );
          }
        }
      }
    }
  }
  return mLayerExtent;
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::getNumberFeatures
//-----------------------------------------------------------------
long SpatialiteDbLayer::getNumberFeatures( bool bUpdateStatistics )
{
  if ( ( isLayerValid() ) && ( bUpdateStatistics ) )
  {
    getLayerExtent( true, true );
  }
  return mNumberFeatures;
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::setSpatialiteAttributeFields
//-----------------------------------------------------------------
// Retrieve the field-names for TABLE OR VIEW
// - for writable VIEWS (SpatialView)
// -> the underlying will be used to set
// --> field-types and default values
// -> an attempt will be made to read the TRIGGERs
// -->  and found const value setting will be displayed as DEFAULT
//-----------------------------------------------------------------
int SpatialiteDbLayer::setSpatialiteAttributeFields()
{
  mAttributeFields.clear();
  mPrimaryKeyAttrs.clear();
  mDefaultValues.clear();
  sqlite3_stmt *stmt = nullptr;
  // The Column-Number of the primary key of the Table and View may be different
  int pk_cid_table = -1;
  int pk_cid_view = -1;
  // Missing entries in 'vector_layers_field_infos' TABLE.
  // Cause could be: when properly registered TABLE or VIEW is empty
  // dbLayer->setLayerInvalid( mLayerName, QString( "GetDbLayersInfo [%1] Missing entries in 'vector_layers_field_infos'. TABLE may be empty." ).arg( sSearchLayer ) );
  QString sql = QStringLiteral( "PRAGMA table_info('%1')" ).arg( mTableName );
  // PRAGMA table_info('pg_grenzkommando_1985')
  // PRAGMA table_info('street_segments_3000')
  // PRAGMA table_info('middle_earth_polygons')
  int i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
  if ( i_rc == SQLITE_OK )
  {
    int cid = 0;    // 0
    QString sName; // 1
    QString sType;  // 2
    // bool notNull;      // 3
    QString sDefaultValue;  // 4
    // int isPK;  // 5
    while ( sqlite3_step( stmt ) == SQLITE_ROW )
    {
      if ( ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL ) &&
           ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL ) &&
           ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL ) &&
           ( sqlite3_column_type( stmt, 3 ) != SQLITE_NULL ) &&
           ( sqlite3_column_type( stmt, 5 ) != SQLITE_NULL ) )
      {
        cid = sqlite3_column_int( stmt, 0 );
        sName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
        sType = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 2 ) );
        sDefaultValue = QString();
        if ( sqlite3_column_type( stmt, 4 ) != SQLITE_NULL )
        {
          // Not an error if NULL (which it often is)
          sDefaultValue = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 4 ) );
        }
        if ( mLayerType == SpatialiteDbInfo::SpatialView )
        {
          // mPrimaryKey was retrieved from 'views_geometry_columns'
          if ( mPrimaryKey == sName )
          {
            mPrimaryKeyAttrs << mAttributeFields.size();
            // Error if primary Key was not found
            pk_cid_view = cid;
          }
        }
        if ( sName.toLower() != mGeometryColumn.toLower() )
        {
          QVariant defaultVariant; // Will be set during GetSpatialiteQgsField.
          QgsField column_field = SpatialiteDbLayer::GetSpatialiteQgsField( sName, sType, sDefaultValue, defaultVariant );
          if ( sqlite3_column_int( stmt, 3 ) == 1 )
          {
            // column 4[3] tells us if the field is nullable. Should use that info...
            QgsFieldConstraints constraints = column_field.constraints();
            constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
            column_field.setConstraints( constraints );
          }
          if ( sqlite3_column_int( stmt, 5 ) == 1 )
          {
            if ( mLayerType == SpatialiteDbInfo::SpatialTable )
            {
              // Only if unique pk [SpatialView will almost never have this set here as 1]
              if ( mPrimaryKey.isEmpty() )
              {
                mPrimaryKey = sName;
                pk_cid_table = cid;
                mPrimaryKeyAttrs << mAttributeFields.size();
              }
              else
              {
                mPrimaryKey.clear();
                mPrimaryKeyAttrs.clear();
              }
            }
          }
          // As done in QgsPostgresProvider::loadFields
          mDefaultValues.insert( mAttributeFields.size(), defaultVariant );
          mAttributeFields.append( column_field );
        }
      }
    }
    sqlite3_finalize( stmt );
  }
  if ( mLayerType == SpatialiteDbInfo::SpatialView )
  {
    if ( pk_cid_view < 0 )
    {
      mIsLayerValid = false;
      QString sError = QString( "-E-> SpatialiteDbLayer::setSpatialiteAttributeFields[%1] Layername[%2] view_rowid[%3] from 'views_geometry_columns',  was not found in the view as a column." ).arg( mLayerTypeString ).arg( mLayerName ).arg( mPrimaryKey );
      mErrors.insert( mLayerName, sError );
      return 0;
    }
    mPrimaryKeyCId = pk_cid_view;
  }

  if ( mLayerType == SpatialiteDbInfo::SpatialTable )
  {
    mPrimaryKeyCId = pk_cid_table;
    sql = QStringLiteral( "SELECT " );
    sql += QStringLiteral( "(SELECT sql FROM sqlite_master WHERE type='table' AND name='%1'))," ).arg( mTableName );
    sql += QStringLiteral( "(SELECT ROWID FROM '%1' LIMIT 1)" ).arg( mTableName );
    i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
        {
          QString sqlTable = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
          // extract definition
          QRegularExpression re( QStringLiteral( "\\((.*)\\)" ) );
          QRegularExpressionMatch match = re.match( sqlTable );
          if ( match.hasMatch() )
          {
            QString matched = match.captured( 1 );
            Q_FOREACH ( QString field, matched.split( ',' ) )
            {
              field = field.trimmed();
              QString fieldName = field.left( field.indexOf( ' ' ) );
              QString definition = field.mid( field.indexOf( ' ' ) + 1 );
              int fieldIndex = mAttributeFields.lookupField( fieldName );
              if ( fieldIndex >= 0 )
              {
                QgsField column_field = mAttributeFields.at( fieldIndex );
                QgsFieldConstraints constraints = column_field.constraints();
                if ( definition.contains( "unique", Qt::CaseInsensitive ) || definition.contains( "primary key", Qt::CaseInsensitive ) )
                  constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
                if ( definition.contains( "not null", Qt::CaseInsensitive ) || definition.contains( "primary key", Qt::CaseInsensitive ) )
                  constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
                column_field.setConstraints( constraints );
                mAttributeFields[fieldIndex] = column_field;
              }
            }
          }
        }
        if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
        {
          mPrimaryKey.clear();
          mPrimaryKey = QStringLiteral( "ROWID" );
          mPrimaryKeyAttrs.clear();
        }
      }
      sqlite3_finalize( stmt );
    }
  }
  if ( ( mLayerType == SpatialiteDbInfo::SpatialView ) && ( !mLayerReadOnly ) && ( !mViewTableName.isEmpty() ) )
  {
    //-----------------------------------------------------------------
    // PRAGMA table_info('segments_ortsteile_1938') ; PRAGMA table_info('berlin_admin_segments') ;
    // Notes: PRAGMA table_info will rarely contain useful information for notnull or default-values for Views
    // This information can ONLY be retrieved from the underlining TABLE, WHEN they share the same column name.
    // Since with SpatialViews the underlineing table must be registered, we will use that
    //-----------------------------------------------------------------
    sql = QStringLiteral( "PRAGMA table_info('%1')" ).arg( mViewTableName );
    i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      int fieldIndex;
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 3 ) != SQLITE_NULL ) )
        {
          QString sName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
          fieldIndex = mAttributeFields.indexFromName( sName );
          if ( fieldIndex < 0 )
          {
            // Active mGeometryColumn [or unknown field] should be ignored
            continue;
          }
          QString sType = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 2 ) );
          QString sDefaultValue;  // 4
          if ( sqlite3_column_type( stmt, 4 ) != SQLITE_NULL )
          {
            sDefaultValue = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 4 ) );
          }
          QVariant defaultVariant; // Will be set during GetSpatialiteQgsField.
          QgsField column_field = SpatialiteDbLayer::GetSpatialiteQgsField( sName, sType, sDefaultValue, defaultVariant );
          // Replace the (possibly) changed defaut-value
          mDefaultValues.insert( fieldIndex, defaultVariant );
          if ( sqlite3_column_int( stmt, 3 ) == 1 )
          {
            // column 4[3] tells us if the field is nullable. Should use that info...
            QgsFieldConstraints constraints = column_field.constraints();
            constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
            column_field.setConstraints( constraints );
            // mAttributeFields.at( fieldIndex ).setConstraints( constraints );
          }
          //-------------------------------------------------------------
          // Set the types and the comment based on the TABLE value
          // mAttributeFields.at( fieldIndex ) : seem to be read only
          //-------------------------------------------------------------
          /*
          mAttributeFields.at( fieldIndex ).setType( column_field.type() );
          mAttributeFields.at( fieldIndex ).setTypeName( column_field.typeName() );
          if ( column_field.typeName() == "TEXT" )
          {
            mAttributeFields.at( fieldIndex ).setSubType( column_field.subType() );
          }
          mAttributeFields.at( fieldIndex ).setComment( QString( column_field.comment() ) );
          */
          mAttributeFields[fieldIndex] = column_field;
        }
      }
      sqlite3_finalize( stmt );
    }
    if ( !mViewTableName.isEmpty() )
    {
      if ( ( mTriggerInsert ) || ( mTriggerUpdate ) )
      {
        //-------------------------------------------------------------
        // Try to extract any const values set in the Triggers
        // - set found value as DEFAULT
        //-------------------------------------------------------------
        QMap<QString, QString> view_defaults;
        QString sqlInsert;
        QString sqlUpdate;
        QString sql_base = QStringLiteral( "(SELECT sql FROM  sqlite_master WHERE (type = 'trigger' AND tbl_name = '%1' AND (instr(upper(sql),'INSTEAD OF %2') > 0)))" );
        sql = QStringLiteral( "SELECT " );
        if ( mTriggerInsert )
        {
          sql += sql_base.arg( mTableName ).arg( "INSERT" );
          if ( mTriggerUpdate )
          {
            sql += ",";
          }
        }
        if ( mTriggerUpdate )
        {
          sql += sql_base.arg( mTableName ).arg( "UPDATE" );
        }
        i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
        if ( i_rc == SQLITE_OK )
        {
          while ( sqlite3_step( stmt ) == SQLITE_ROW )
          {
            if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
            {
              QString sqlTable = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
              if ( mTriggerInsert )
              {
                sqlInsert = sqlTable;
              }
              else
              {
                sqlUpdate = sqlTable;
              }
            }
            if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
            {
              sqlUpdate = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
            }
          }
          sqlite3_finalize( stmt );
          if ( !sqlUpdate.isEmpty() )
          {
            QRegularExpression re( QStringLiteral( "SET (.*) WHERE" ), QRegularExpression::CaseInsensitiveOption );
            // Note: remove line feeds (new line), tabs and carriage returns, plus multiple blanks !
            QString sqlReplaced = sqlUpdate.remove( QRegExp( "[\\n\\t\\r]" ) ).replace( "  ", " " ).replace( "VALUES (", "VALUES(" );
            QRegularExpressionMatch match = re.match( sqlReplaced );
            if ( ( match.hasMatch() ) && ( match.lastCapturedIndex() == 1 ) )
            {
              QStringList sa_fields = match.captured( 1 ).split( "," );
              if ( sa_fields.size() )
              {
                for ( int i = 0 ; i < sa_fields.size(); i++ )
                {
                  QString sTrimmed = sa_fields[i].trimmed();
                  if ( ( !sTrimmed.toUpper().contains( "NEW." ) ) ||
                       ( !sTrimmed.startsWith( "--" ) ) ||
                       ( sTrimmed.toUpper().contains( "CASE WHEN NEW." ) ) )
                  {
                    //  gcp_type = CASE WHEN NEW.gcp_type IS NULL THEN 700 ELSE NEW.gcp_type END,
                    QStringList sa_values = sTrimmed.split( "=" );
                    // Ignore any values that start with a '(' [Sub-Query]
                    if ( ( sa_values.size() ) && ( sa_values.size() > 1 ) &&
                         ( !sa_values.at( 1 ).trimmed().startsWith( "(" ) ) &&
                         ( !sa_values.at( 1 ).trimmed().startsWith( "ST_" ) ) &&
                         ( !sa_values.at( 1 ).trimmed().toUpper().startsWith( "NEW." ) ) )
                    {
                      QString sField = sa_values.at( 0 ).trimmed(); // Result here: gcp_type
                      sTrimmed = sa_values.at( 1 ).trimmed();
                      if ( ( sTrimmed.toUpper().startsWith( "CASE WHEN NEW." ) ) &&
                           ( sTrimmed.toUpper().contains( "IS NULL THEN" ) ) &&
                           ( sTrimmed.toUpper().contains( "ELSE NEW." ) ) )
                      {
                        //  CASE WHEN NEW.gcp_type IS NULL THEN 700 ELSE NEW.gcp_type END,
                        QRegularExpression re_case( QStringLiteral( "IS NULL THEN (.*) ELSE NEW." ), QRegularExpression::CaseInsensitiveOption );
                        QRegularExpressionMatch match_case = re_case.match( sTrimmed );
                        if ( ( match_case.hasMatch() ) && ( match_case.lastCapturedIndex() == 1 ) )
                        {
                          sTrimmed = match_case.captured( 1 ); // Result here: 700
                        }
                      }
                      // remove any possible garbage, such as comments etc. at the end of the line
                      sa_values = sTrimmed.split( QRegExp( "\\s+" ) );
                      QString sValue = sa_values.at( 0 ).trimmed();
                      if ( sField.startsWith( "--" ) )
                      {
                        // remove any possible garbage, such as comments etc. that was before the line
                        sa_values = sField.trimmed().split( QRegExp( "\\s+" ) );
                        sField = sa_values.at( sa_values.size() - 1 );
                      }
                      // The default value of a writable view may be different than that of the underlining table.
                      view_defaults.insert( sField, sValue.remove( "\"" ) );
                    }
                  }
                }
              }
            }
          }
          if ( !sqlInsert.isEmpty() )
          {
            // Warning: the order of the given value MUST match the order of the fields !
            // - this can be tricky, depending on how the sql was written. Thus UPDATE will first be used (when found), since it is easier to parse.
            QRegularExpression re( QStringLiteral( "\\((.*)\\) VALUES\\((.*)\\)" ), QRegularExpression::CaseInsensitiveOption );
            // Note: remove line feeds (new line), tabs and carriage returns, plus multiple blanks !
            QString sqlReplaced = sqlInsert.remove( QRegExp( "[\\n\\t\\r]" ) ).replace( "  ", " " ).replace( "VALUES (", "VALUES(" );
            QRegularExpressionMatch match = re.match( sqlReplaced );
            if ( ( match.hasMatch() ) && ( match.lastCapturedIndex() == 2 ) )
            {
              QStringList sa_fields = match.captured( 1 ).split( "," );
              if ( sa_fields.size() )
              {
                QStringList sa_values = match.captured( 2 ).split( "," );
                if ( sa_fields.size() == sa_values.size() )
                {
                  for ( int i = 0 ; i < sa_values.size(); i++ )
                  {
                    QString sTrimmed = sa_values.at( i ).trimmed();
                    if ( ( !sTrimmed.toUpper().startsWith( "NEW." ) ) &&
                         ( !sTrimmed.startsWith( "--" ) ) &&
                         ( sTrimmed.toUpper().contains( "CASE WHEN NEW." ) ) )
                    {
                      if ( ( sTrimmed.toUpper().startsWith( "CASE WHEN NEW." ) ) &&
                           ( sTrimmed.toUpper().contains( "IS NULL THEN" ) ) &&
                           ( sTrimmed.toUpper().contains( "ELSE NEW." ) ) )
                      {
                        // CASE WHEN NEW.gcp_type IS NULL THEN 700 ELSE NEW.gcp_type END,
                        QRegularExpression re_case( QStringLiteral( "IS NULL THEN (.*) ELSE NEW." ), QRegularExpression::CaseInsensitiveOption );
                        QRegularExpressionMatch match_case = re_case.match( sTrimmed );
                        if ( ( match_case.hasMatch() ) && ( match_case.lastCapturedIndex() == 1 ) )
                        {
                          sTrimmed = match_case.captured( 1 ).trimmed(); // Result here: 700
                        }
                      }
                      QString sField = sa_fields[i].trimmed();
                      QString sValue = sTrimmed.remove( "\"" );
                      // The default value of a writable view may be different than that of the underlining table.
                      // If we already have this value from the UPDATE TRIGGER, then we can ignore this.
                      if ( !view_defaults.value( sField ).isEmpty() )
                      {
                        view_defaults.insert( sField, sValue );
                      }
                    }
                  }
                }
              }
            }
          }
        }
        for ( QMap<QString, QString>::iterator itFields = view_defaults.begin(); itFields != view_defaults.end(); ++itFields )
        {
          int fieldIndex = mAttributeFields.lookupField( itFields.key() );
          if ( fieldIndex >= 0 )
          {
            QVariant defaultVariant; // Will be set during GetSpatialiteQgsField.
            QgsField column_field = SpatialiteDbLayer::GetSpatialiteQgsField( itFields.key(), mAttributeFields.at( fieldIndex ).typeName(), itFields.value(), defaultVariant );
            // Replace the (possibly) changed defaut-value
            mDefaultValues.insert( fieldIndex, defaultVariant );
            // mAttributeFields.at( fieldIndex ) : seem to be read only
            // mAttributeFields.at( fieldIndex ).setComment( QString( column_field.comment() ) );
            mAttributeFields[fieldIndex] = column_field;
          }
        }
      }
    }
  }
  return mAttributeFields.size();
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::GetLayerSettings
//-----------------------------------------------------------------
bool SpatialiteDbLayer::GetLayerSettings()
{
  bool bRc = false;
  if ( isLayerValid() )
  {
    int i_rc = 0;
    sqlite3_stmt *stmt = nullptr;
    QString sSearchTable = mTableName;
    QString sql;
    if ( mLayerType == SpatialiteDbInfo::SpatialView )
    {
      //-----------------------------------------
      //-- Retrieve from View ----------------
      //-- mPrimaryKey
      //-- mViewTableName
      //-- mViewTableGeometryColumn
      //-----------------------------------------
      // SELECT view_rowid, f_table_name, f_geometry_column FROM views_geometry_columns WHERE (view_name="positions_1925")
      sql = QStringLiteral( "SELECT view_rowid, f_table_name, f_geometry_column FROM views_geometry_columns WHERE ((view_name='%1') AND (view_geometry='%2'))" ).arg( mTableName ).arg( mGeometryColumn );
      i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
      if ( i_rc == SQLITE_OK )
      {
        while ( sqlite3_step( stmt ) == SQLITE_ROW )
        {
          if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
          {
            QString viewRowid = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
            if ( mPrimaryKey.isEmpty() )
            {
              mPrimaryKey = viewRowid;
            }
          }
          if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
          {
            mViewTableName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
            sSearchTable = mViewTableName;
          }
          if ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL )
          {
            mViewTableGeometryColumn = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 2 ) );
          }
        }
        sqlite3_finalize( stmt );
      }
      if ( !mViewTableName.isEmpty() )
      {
        mLayerViewTableName = mViewTableName;
        if ( !mViewTableGeometryColumn.isEmpty() )
        {
          mLayerViewTableName = QString( "%1(%2)" ).arg( mLayerViewTableName ).arg( mViewTableGeometryColumn );
        }
      }
    }
    //-----------------------------------------
    //-- Fill ----------------------------------
    //-- mAttributeFields
    //-- mDefaultValues
    //-----------------------------------------
    if ( setSpatialiteAttributeFields() <= 0 )
    {
      mIsLayerValid = false;
    }
    //-----------------------------------------
    //-----------------------------------------
    //---setting EnabledCapabilities---------
    //-----------------------------------------
    prepare();
    bRc = isLayerValid();
  }
  else
  {
    mErrors.insert( mLayerName, QString( "SpatialiteDbLayer::GetLayerSettings: called with !mIsLayerValid [%1]" ).arg( mLayerName ) );
  }
  return bRc;
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::GetViewTriggers
//-----------------------------------------------------------------
bool SpatialiteDbLayer::GetViewTriggers()
{
  if ( ( mDbSpatialiteInfo ) && ( isLayerValid() ) )
  {
    if ( ( mLayerType == SpatialiteDbInfo::SpatialTable ) || ( mLayerType == SpatialiteDbInfo::VirtualShape ) )
    {
      if ( mLayerType == SpatialiteDbInfo::SpatialTable )
      {
        mLayerReadOnly = 0; // default is 1
      }
      return mLayerReadOnly;
    }
    sqlite3_stmt *stmt = nullptr;
    QString sql;
    QString sql_base = QStringLiteral( "(SELECT Exists(SELECT rootpage FROM  sqlite_master WHERE (type = 'trigger' AND tbl_name = '%1' AND (instr(upper(sql),'INSTEAD OF %2') > 0))))" );
    sql = QStringLiteral( "SELECT " );
    sql += sql_base.arg( mTableName ).arg( "INSERT" ) + ",";
    sql += sql_base.arg( mTableName ).arg( "UPDATE" ) + ",";
    sql += sql_base.arg( mTableName ).arg( "DELETE" );
    int ret = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( ret == SQLITE_OK )
    {
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
        {
          if ( sqlite3_column_int( stmt, 0 ) == 1 )
          {
            mTriggerInsert = true;
            mLayerReadOnly = false;
          }
        }
        if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
        {
          if ( sqlite3_column_int( stmt, 1 ) == 1 )
          {
            mTriggerUpdate = true;
            mLayerReadOnly = false;
          }
        }
        if ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL )
        {
          if ( sqlite3_column_int( stmt, 2 ) == 1 )
          {
            mTriggerDelete = true;
            mLayerReadOnly = false;
          }
        }
      }
    }
    return mLayerReadOnly;
  }
  return false;
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::GetSpatialiteQgsField [static]
//-----------------------------------------------------------------
QgsField SpatialiteDbLayer::GetSpatialiteQgsField( const QString sName, const QString sType, QString sDefaultValue, QVariant &defaultVariant )
{
  QgsField column_field;
  column_field.setName( sName );
  QgsWkbTypes::Type geometryType = SpatialiteDbLayer::GetGeometryTypeLegacy( sType );
  if ( geometryType != QgsWkbTypes::Unknown )
  {
    defaultVariant = sDefaultValue.toUtf8().constData();
    column_field.setType( QVariant::ByteArray );
    column_field.setTypeName( sType );
    return column_field;
  }
  else
  {
    QString type = "TEXT";
    QString comment_text = QString();
    if ( sDefaultValue.contains( "NULL", Qt::CaseInsensitive ) )
    {
      sDefaultValue = QString(); // No default valiue
    }
    QVariant::Type fieldType = QVariant::String; // default: SQLITE_TEXT
    if ( sType == "INTEGER" )
    {
      fieldType = QVariant::LongLong;
      type = "INTEGER";
      defaultVariant = sDefaultValue.toLongLong();
      if ( sDefaultValue.size() > 0 )
      {
        comment_text = QString( "DEFAULT=%1" ).arg( sDefaultValue );
      }
    }
    else if ( sType == "DOUBLE" )
    {
      fieldType = QVariant::Double;
      type = "DOUBLE";
      defaultVariant = sDefaultValue.toDouble();
      if ( sDefaultValue.size() > 0 )
      {
        comment_text = QString( "DEFAULT=%1" ).arg( sDefaultValue );
      }
    }
    else if ( sType == "DATE" )
    {
      fieldType = QVariant::Date;
      type = "DATE";
      QString date_DefaultValue = sDefaultValue;
      // Detailed Description: The minimum value for QDateTimeEdit is 14 September 1752. You can change this by calling setMinimumDate(), taking into account that the minimum value for QDate is 2 January 4713BC.
      // minimumDate: By default, this property contains a date that refers to September 14, 1752. The minimum date must be at least the first day in year 100, otherwise setMinimumDate() has no effect.
      date_DefaultValue.replace( 0x22, "" ).replace( 0x27, "" ); // remove any " or ' - 1752-09-14 minimumDateTime
      defaultVariant = QDate::fromString( date_DefaultValue, Qt::ISODate ); // "YYYY-MM-DD"=Qt::ISODate
      if ( date_DefaultValue.size() > 0 )
      {
        comment_text = QString( "DEFAULT='%1'" ).arg( date_DefaultValue );
      }
    }
    else if ( sType == "DATETIME" )
    {
      fieldType = QVariant::DateTime;
      type = "DATETIME";
      QString date_DefaultValue = sDefaultValue;
      date_DefaultValue.replace( 0x22, "" ).replace( 0x27, "" ); // remove any " or '
      defaultVariant = QDate::fromString( date_DefaultValue, Qt::ISODate ); // "YYYY-MM-DD"=Qt::ISODate
      if ( date_DefaultValue.size() > 0 )
      {
        comment_text = QString( "DEFAULT='%1'" ).arg( date_DefaultValue );
      }
    }
    else if ( sType == "TIME" )
    {
      fieldType = QVariant::Time;
      type = "TIME";
      QString date_DefaultValue = sDefaultValue;
      date_DefaultValue.replace( 0x22, "" ).replace( 0x27, "" ); // remove any " or '
      defaultVariant = QDate::fromString( date_DefaultValue, Qt::ISODate ); // "YYYY-MM-DD"=Qt::ISODate
      if ( date_DefaultValue.size() > 0 )
      {
        comment_text = QString( "DEFAULT='%1'" ).arg( date_DefaultValue );
      }
    }
    else if ( sType == "BLOB" )
    {
      fieldType = QVariant::ByteArray;
      type = "BLOB";
      defaultVariant = sDefaultValue.toUtf8().constData();
    }
    else
    {
      QString defaultString = sDefaultValue;
      if ( defaultString.startsWith( '\'' ) )
        defaultString = defaultString.remove( 0, 1 );
      if ( defaultString.endsWith( '\'' ) )
        defaultString.chop( 1 );
      defaultString.replace( "''", "'" );
      if ( defaultString.size() > 0 )
      {
        comment_text = QString( "DEFAULT='%1'" ).arg( defaultString );
      }
      defaultVariant = defaultString;
      // if the type seems unknown, fix it with what we actually have
      SpatialiteDbInfo::TypeSubType typeSubType = SpatialiteDbInfo::GetVariantType( sType );
      column_field.setType( typeSubType.first );
      column_field.setSubType( typeSubType.second );
    }
    if ( type != "TEXT" )
    {
      column_field.setType( fieldType );
    }
    column_field.setTypeName( type );
    column_field.setComment( QString( comment_text ) );
    // column_field.convertCompatible( defaultVariant );
    return column_field;
  }
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::GetGeometryTypeLegacy
//-----------------------------------------------------------------
QgsWkbTypes::Type SpatialiteDbLayer::GetGeometryTypeLegacy( const QString spatialiteGeometryType, const QString spatialiteGeometryDimension )
{
  QgsWkbTypes::Type geometryType = QgsWkbTypes::Unknown;
  int geometryDimension = GAIA_XY;
  QString sGeometryDimension = spatialiteGeometryDimension;
  if ( sGeometryDimension.isEmpty() )
  {
    sGeometryDimension = "XY";
  }
  if ( spatialiteGeometryType == "POINT" )
  {
    geometryType = QgsWkbTypes::Point;
  }
  else if ( spatialiteGeometryType == "POINTZ" )
  {
    sGeometryDimension = "XYZ";
    geometryType = QgsWkbTypes::Point;
  }
  else if ( spatialiteGeometryType == "POINTM" )
  {
    geometryType = QgsWkbTypes::Point;
    sGeometryDimension = "XYM";
  }
  else if ( spatialiteGeometryType == "POINTZM" )
  {
    geometryType = QgsWkbTypes::Point;
    sGeometryDimension = "XYZM";
  }
  else if ( spatialiteGeometryType == "MULTIPOINT" )
  {
    geometryType = QgsWkbTypes::MultiPoint;
  }
  else if ( spatialiteGeometryType == "MULTIPOINTZ" )
  {
    geometryType = QgsWkbTypes::MultiPoint;
    sGeometryDimension = "XYZ";
  }
  else if ( spatialiteGeometryType == "MULTIPOINTM" )
  {
    geometryType = QgsWkbTypes::MultiPoint;
    sGeometryDimension = "XYM";
  }
  else if ( spatialiteGeometryType == "MULTIPOINTZM" )
  {
    geometryType = QgsWkbTypes::MultiPoint;
    sGeometryDimension = "XYZM";
  }
  else if ( spatialiteGeometryType == "LINESTRING" )
  {
    geometryType = QgsWkbTypes::LineString;
  }
  else if ( spatialiteGeometryType == "LINESTRINGZ" )
  {
    geometryType = QgsWkbTypes::LineString;
    sGeometryDimension = "XYZ";
  }
  else if ( spatialiteGeometryType == "LINESTRINGM" )
  {
    geometryType = QgsWkbTypes::LineString;
    sGeometryDimension = "XYM";
  }
  else if ( spatialiteGeometryType == "LINESTRINGZM" )
  {
    geometryType = QgsWkbTypes::LineString;
    sGeometryDimension = "XYZM";
  }
  else if ( spatialiteGeometryType == "MULTILINESTRING" )
  {
    geometryType = QgsWkbTypes::MultiLineString;
  }
  else if ( spatialiteGeometryType == "MULTILINESTRINGZ" )
  {
    geometryType = QgsWkbTypes::MultiLineString;
    sGeometryDimension = "XYZ";
  }
  else if ( spatialiteGeometryType == "MULTILINESTRINGM" )
  {
    geometryType = QgsWkbTypes::MultiLineString;
    sGeometryDimension = "XYM";
  }
  else if ( spatialiteGeometryType == "MULTILINESTRINGZM" )
  {
    geometryType = QgsWkbTypes::MultiLineString;
    sGeometryDimension = "XYZM";
  }
  else if ( spatialiteGeometryType == "POLYGON" )
  {
    geometryType = QgsWkbTypes::Polygon;
  }
  else if ( spatialiteGeometryType == "POLYGONZ" )
  {
    geometryType = QgsWkbTypes::Polygon;
    sGeometryDimension = "XYZ";
  }
  else if ( spatialiteGeometryType == "POLYGONM" )
  {
    geometryType = QgsWkbTypes::Polygon;
    sGeometryDimension = "XYM";
  }
  else if ( spatialiteGeometryType == "POLYGONZM" )
  {
    geometryType = QgsWkbTypes::Polygon;
    sGeometryDimension = "XYZM";
  }
  else if ( spatialiteGeometryType == "MULTIPOLYGON" )
  {
    geometryType = QgsWkbTypes::MultiPolygon;
  }
  else if ( spatialiteGeometryType == "MULTIPOLYGONZ" )
  {
    geometryType = QgsWkbTypes::MultiPolygon;
    sGeometryDimension = "XYZ";
  }
  else if ( spatialiteGeometryType == "MULTIPOLYGONM" )
  {
    geometryType = QgsWkbTypes::MultiPolygon;
    sGeometryDimension = "XYM";
  }
  else if ( spatialiteGeometryType == "MULTIPOLYGONZM" )
  {
    geometryType = QgsWkbTypes::MultiPolygon;
    sGeometryDimension = "XYZM";
  }
  else if ( spatialiteGeometryType == "GEOMETRYCOLLECTION" )
  {
    geometryType = QgsWkbTypes::GeometryCollection;
  }
  else if ( spatialiteGeometryType == "GEOMETRYCOLLECTIONZ" )
  {
    geometryType = QgsWkbTypes::GeometryCollection;
    sGeometryDimension = "XYZ";
  }
  else if ( spatialiteGeometryType == "GEOMETRYCOLLECTIONM" )
  {
    geometryType = QgsWkbTypes::GeometryCollection;
    sGeometryDimension = "XYM";
  }
  else if ( spatialiteGeometryType == "GEOMETRYCOLLECTIONZM" )
  {
    geometryType = QgsWkbTypes::GeometryCollection;
    sGeometryDimension = "XYZM";
  }
  if ( sGeometryDimension == "XYZ" )
  {
    geometryDimension = GAIA_XY_Z;
  }
  else if ( sGeometryDimension == "XYM" )
  {
    geometryDimension = GAIA_XY_M;
  }
  else if ( sGeometryDimension == "XYZM" )
  {
    geometryDimension = GAIA_XY_Z_M;
  }
  switch ( geometryDimension )
  {
    case GAIA_XY_Z:
    {
      switch ( geometryType )
      {
        default:
        case QgsWkbTypes::Point:
          geometryType = QgsWkbTypes::PointZ;
          break;
        case QgsWkbTypes::MultiPoint:
          geometryType = QgsWkbTypes::MultiPointZ;
          break;
        case QgsWkbTypes::LineString:
          geometryType = QgsWkbTypes::LineStringZ;
          break;
        case QgsWkbTypes::MultiLineString:
          geometryType = QgsWkbTypes::MultiLineStringZ;
          break;
        case QgsWkbTypes::Polygon:
          geometryType = QgsWkbTypes::PolygonZ;
          break;
        case QgsWkbTypes::MultiPolygon:
          geometryType = QgsWkbTypes::MultiPolygonZ;
          break;
        case QgsWkbTypes::GeometryCollection:
          geometryType = QgsWkbTypes::GeometryCollectionZ;
          break;
      }
    }
    break;
    case GAIA_XY_M:
    {
      switch ( geometryType )
      {
        default:
        case QgsWkbTypes::Point:
          geometryType = QgsWkbTypes::PointM;
          break;
        case QgsWkbTypes::MultiPoint:
          geometryType = QgsWkbTypes::MultiPointM;
          break;
        case QgsWkbTypes::LineString:
          geometryType = QgsWkbTypes::LineStringM;
          break;
        case QgsWkbTypes::MultiLineString:
          geometryType = QgsWkbTypes::MultiLineStringM;
          break;
        case QgsWkbTypes::Polygon:
          geometryType = QgsWkbTypes::PolygonM;
          break;
        case QgsWkbTypes::MultiPolygon:
          geometryType = QgsWkbTypes::MultiPolygonM;
          break;
        case QgsWkbTypes::GeometryCollection:
          geometryType = QgsWkbTypes::GeometryCollectionM;
          break;
      }
    }
    break;
    case GAIA_XY_Z_M:
    {
      switch ( geometryType )
      {
        default:
        case QgsWkbTypes::Point:
          geometryType = QgsWkbTypes::PointZM;
          break;
        case QgsWkbTypes::MultiPoint:
          geometryType = QgsWkbTypes::MultiPointZM;
          break;
        case QgsWkbTypes::LineString:
          geometryType = QgsWkbTypes::LineStringZM;
          break;
        case QgsWkbTypes::MultiLineString:
          geometryType = QgsWkbTypes::MultiLineStringZM;
          break;
        case QgsWkbTypes::Polygon:
          geometryType = QgsWkbTypes::PolygonZM;
          break;
        case QgsWkbTypes::MultiPolygon:
          geometryType = QgsWkbTypes::MultiPolygonZM;
          break;
        case QgsWkbTypes::GeometryCollection:
          geometryType = QgsWkbTypes::GeometryCollectionZM;
          break;
      }
    }
    break;
  }
  return geometryType;
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::GetGeometryType
//-----------------------------------------------------------------
QgsWkbTypes::Type SpatialiteDbLayer::GetGeometryType( const int spatialiteGeometryType,   const int spatialiteGeometryDimension )
{
  Q_UNUSED( spatialiteGeometryDimension );
  QgsWkbTypes::Type geometryType = ( QgsWkbTypes::Type )spatialiteGeometryType;
  return geometryType;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteProvider functions
//-----------------------------------------------------------------
// SpatialiteDbLayer::setLayerBandsInfo
//-----------------------------------------------------------------
void SpatialiteDbLayer::setLayerBandsInfo( QStringList layerBandsInfo, QMap<int, QImage> layerBandsHistograms )
{
  mLayerBandsInfo = layerBandsInfo;
  mLayerBandsHistograms = layerBandsHistograms;
  mDefaultImageBackground = QString( "#ffffff" );
  QStringList sa_list_info = mLayerBandsInfo.at( 0 ).split( SpatialiteDbInfo::ParseSeparatorGeneral );
  if ( mLayerBandsInfo.size() > 0 )
  {
    int i_NoDataPixelValue = 255;
    for ( int i = 0; i < mLayerBandsInfo.size(); i++ )
    {
      QStringList sa_list_info = mLayerBandsInfo.at( i ).split( SpatialiteDbInfo::ParseSeparatorGeneral );
      if ( sa_list_info.size() == 8 )
      {
        if ( i == 0 )
        {
          mDefaultImageBackground = QString( "#" );
          int i_PixelNodata = sa_list_info.at( 7 ).toInt();
          mLayerBandsNodata.insert( i, sa_list_info.at( 0 ).toInt() );
          int i_PixelValid = sa_list_info.at( 6 ).toInt();
          int i_PixelImage = i_PixelNodata + i_PixelValid;
          // Calculation based on extent and resolution may differ
          int i_PixelDiff = i_PixelDiff - ( getLayerImageWidth() * getLayerImageHeight() );
          mLayerPixelSizes = QRect( i_PixelNodata, i_PixelValid, i_PixelImage, i_PixelDiff );
        }
        if ( i < 3 )
        {
          // max is '#rrggbb'
          i_NoDataPixelValue = sa_list_info.at( 0 ).toInt();
          // Hex(0) must be shown as '00'
          mDefaultImageBackground += QString( "%1" ).arg( i_NoDataPixelValue, 2, 16, QLatin1Char( '0' ) );
        }
      }
    }
    // '#rr' or '#rrgg' is invalid, must be '#rrggbb'
    if ( mDefaultImageBackground.size() == 5 )
    {
      mDefaultImageBackground += QString( "%1" ).arg( i_NoDataPixelValue, 2, 16, QLatin1Char( '0' ) );
    }
    if ( mDefaultImageBackground.size() == 3 )
    {
      mDefaultImageBackground += QString( "%1%1" ).arg( i_NoDataPixelValue, 2, 16, QLatin1Char( '0' ) );
    }
  }
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::setLayerRasterTypesInfo
//-----------------------------------------------------------------
bool SpatialiteDbLayer::setLayerRasterTypesInfo( QRect layerBandsTileSize, QString sLayerSampleType, QString  sLayerPixelType, QString  sLayerCompressionType )
{
  bool bRc = false;
  mLayerBandsTileSize = layerBandsTileSize;
  mLayerSampleType = sLayerSampleType;
  mLayerPixelTypeString = sLayerPixelType;
  mLayerCompressionType = sLayerCompressionType;
  mLayerPixelType = 16; // RL2_PIXEL_UNKNOWN [0x10 = 10 dec]
  if ( mLayerPixelTypeString ==  QLatin1String( "MONOCHROME" ) )
  {
    mLayerPixelType = 17; // RL2_PIXEL_MONOCHROME [0x11 = 17 dec]
  }
  else if ( mLayerPixelTypeString ==  QLatin1String( "GRAYSCALE" ) )
  {
    mLayerPixelType = 18; // RL2_PIXEL_GRAYSCALE [0x12 = 18 dec]
  }
  else if ( mLayerPixelTypeString ==  QLatin1String( "PALETTE" ) )
  {
    mLayerPixelType = 19; // RL2_PIXEL_PALETTE [0x13 = 19 dec]
  }
  else if ( mLayerPixelTypeString ==  QLatin1String( "RGB" ) )
  {
    mLayerPixelType = 20; // RL2_PIXEL_RGB [0x14 = 20 dec]
  }
  else if ( mLayerPixelTypeString ==  QLatin1String( "MULTIBAND" ) )
  {
    mLayerPixelType = 21; // RL2_PIXEL_MULTIBAND [0x15 = 21 dec]
  }
  else if ( mLayerPixelTypeString ==  QLatin1String( "DATAGRID" ) )
  {
    mLayerPixelType = 22; // RL2_PIXEL_DATAGRID [0x16 = 22 dec]
  }

  if ( ( mLayerSampleType.endsWith( QLatin1String( "-BIT" ) ) ) || ( mLayerSampleType.endsWith( QLatin1String( "INT8" ) ) ) )
  {
    mLayerRasterDataType = Qgis::Byte;
    mLayerRasterDataTypeString = QStringLiteral( "Byte - Eight bit unsigned integer" );
  }
  else if ( mLayerSampleType == QLatin1String( "UINT16" ) )
  {
    mLayerRasterDataType = Qgis::UInt16;
    mLayerRasterDataTypeString = QStringLiteral( "UInt16 - Sixteen bit unsigned integer" );
  }
  else if ( mLayerSampleType == QLatin1String( "UINT32" ) )
  {
    mLayerRasterDataType = Qgis::UInt32;
    mLayerRasterDataTypeString = QStringLiteral( "UInt32 - Thirty two bit unsigned integer" );
  }
  else if ( mLayerSampleType == QLatin1String( "INT16" ) )
  {
    mLayerRasterDataType = Qgis::Int16;
    mLayerRasterDataTypeString = QStringLiteral( "Int16 - Sixteen bit signed integer" );
  }
  else if ( mLayerSampleType == QLatin1String( "INT32" ) )
  {
    mLayerRasterDataType = Qgis::Int32;
    mLayerRasterDataTypeString = QStringLiteral( "Int32 - Thirty two bit signed integer" );
  }
  else if ( mLayerSampleType == QLatin1String( "FLOAT" ) )
  {
    mLayerRasterDataType = Qgis::Float32;
    mLayerRasterDataTypeString = QStringLiteral( "Float32 - Thirty two bit floating point" );
  }
  else if ( mLayerSampleType == QLatin1String( "DOUBLE" ) )
  {
    mLayerRasterDataType = Qgis::Float32;
    mLayerRasterDataTypeString = QStringLiteral( "Float64 - Sixty four bit floating point" );
  }
  else
  {
    mLayerRasterDataType = Qgis::UnknownDataType;
    mLayerRasterDataTypeString = QStringLiteral( "Could not determine raster data type for [%1]" ).arg( mLayerSampleType );
    // ( "CInt16 - Complex Int16 " );
    // ( "CInt32 - Complex Int32 " );
    // ( "CFloat32 - Complex Float32 " );
    // ( "CFloat64 - Complex Float64 " );
  }
  if ( ( getLayerNumBands() > 0 ) && ( mLayerRasterDataType != Qgis::UnknownDataType ) )
  {
    bRc = true;
  }
  return bRc;
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::getLayerGetBandStatistics
//-----------------------------------------------------------------
QStringList SpatialiteDbLayer::getLayerGetBandStatistics()
{
  if ( layerHasRasterlite2() )
  {
    sqlite3_stmt *subStmt = nullptr;
    QString sql;
    int i_rc = 0;
    // This should only be called from the QgsRasterLite2Provider when needed.
    //  Only when a RasterLite2 connection exists [i.e. compiled with RasterLite2 and User has installed]
    QStringList layerBandsInfo;
    QMap<int, QImage> layerBandsHistograms;
    QString subQuery_statistics = QString( "(SELECT statistics FROM raster_coverages WHERE coverage_name ='%1')" ).arg( getCoverageName() );
    QString subQuery_nodata_pixel = QString( "(SELECT nodata_pixel FROM raster_coverages WHERE coverage_name ='%1')" ).arg( getCoverageName() );
    QString sql_base = QStringLiteral( "SELECT RL2_GetPixelValue(%1,%2)||'%3'||" ).arg( subQuery_nodata_pixel ).arg( "ZZZ" ).arg( SpatialiteDbInfo::ParseSeparatorGeneral );
    sql_base += QStringLiteral( "RL2_GetBandStatistics_Min(%1,%2)||'%3'||" ).arg( subQuery_statistics ).arg( "ZZZ" ).arg( SpatialiteDbInfo::ParseSeparatorGeneral );
    sql_base += QStringLiteral( "RL2_GetBandStatistics_Max(%1,%2)||'%3'||" ).arg( subQuery_statistics ).arg( "ZZZ" ).arg( SpatialiteDbInfo::ParseSeparatorGeneral );
    sql_base += QStringLiteral( "RL2_GetBandStatistics_Avg(%1,%2)||'%3'||" ).arg( subQuery_statistics ).arg( "ZZZ" ).arg( SpatialiteDbInfo::ParseSeparatorGeneral );
    sql_base += QStringLiteral( "RL2_GetBandStatistics_Var(%1,%2)||'%3'||" ).arg( subQuery_statistics ).arg( "ZZZ" ).arg( SpatialiteDbInfo::ParseSeparatorGeneral );
    sql_base += QStringLiteral( "RL2_GetBandStatistics_StdDev(%1,%2)||'%3'||" ).arg( subQuery_statistics ).arg( "ZZZ" ).arg( SpatialiteDbInfo::ParseSeparatorGeneral );
    // Note: RL2_GetRasterStatistics_ValidPixelsCount and NoDataPixelsCount has no band parameter
    sql_base += QStringLiteral( "RL2_GetRasterStatistics_ValidPixelsCount(%1)||'%2'||" ).arg( subQuery_statistics ).arg( SpatialiteDbInfo::ParseSeparatorGeneral );
    sql_base += QStringLiteral( "RL2_GetRasterStatistics_NoDataPixelsCount(%1)," ).arg( subQuery_statistics );
    sql_base += QStringLiteral( "RL2_GetBandStatistics_Histogram(%1,%2)" ).arg( subQuery_statistics ).arg( "ZZZ" );
    for ( int i = 0; i < getLayerNumBands(); i++ )
    {
      sql = sql_base;
      sql.replace( "ZZZ", QString( "%1" ).arg( i ) );
      i_rc = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &subStmt, nullptr );
      // QgsDebugMsgLevel( QString( "SpatialiteDbLayer::getLayerGetBandStatisticso ::RL2_GetBandStatistics[%3] i_rc=%2 sql[%1]" ).arg( sql ).arg( i_rc ).arg(i), 4);
      if ( i_rc == SQLITE_OK )
      {
        while ( sqlite3_step( subStmt ) == SQLITE_ROW )
        {
          if ( sqlite3_column_type( subStmt, 0 ) != SQLITE_NULL )
          {
            layerBandsInfo.append( QString::fromUtf8( ( const char * ) sqlite3_column_text( subStmt, 0 ) ) );
          }
          if ( sqlite3_column_type( subStmt, 1 ) != SQLITE_NULL )
          {
            QByteArray imageData;
            QImage imageResult;
            imageData.prepend( ( const char * )sqlite3_column_blob( subStmt, 1 ), sqlite3_column_bytes( subStmt, 1 ) );
            imageResult.loadFromData( imageData );
            layerBandsHistograms.insert( i, imageResult );
          }
        }
        sqlite3_finalize( subStmt );
      }
    }
    setLayerBandsInfo( layerBandsInfo, layerBandsHistograms );
  }
  return mLayerBandsInfo;
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::addLayerFeatures
//-----------------------------------------------------------------
bool SpatialiteDbLayer::addLayerFeatures( QgsFeatureList &flist, QgsFeatureSink::Flags flags, QString &errorMessage )
{
  int ret = SQLITE_ABORT; // Container not supported
  if ( isLayerSpatialite() )
  {
    sqlite3_stmt *stmt = nullptr;
    char *errMsg = nullptr;
    bool toCommit = false;
    QString sql;
    QString values;
    QString separator;
    int ia;
    if ( flist.isEmpty() )
    {
      return true;
    }
    QgsAttributes attributevec = flist[0].attributes();
    ret = sqlite3_exec( dbSqliteHandle(), "BEGIN", nullptr, nullptr, &errMsg );
    if ( ret == SQLITE_OK )
    {
      toCommit = true;
      sql = QStringLiteral( "INSERT INTO %1(" ).arg( QgsSpatiaLiteUtils::quotedIdentifier( getTableName() ) );
      values = QStringLiteral( ") VALUES (" );
      separator = QLatin1String( "" );
      if ( !getGeometryColumn().isEmpty() )
      {
        sql += separator + QgsSpatiaLiteUtils::quotedIdentifier( getGeometryColumn() );
        values += separator + geomParam();
        separator = ',';
      }
      for ( int i = 0; i < attributevec.count(); ++i )
      {
        if ( i >= getAttributeFields().count() )
        {
          continue;
        }
        QString fieldname = getAttributeFields().at( i ).name();
        // do not write the Primary Key, will be set with auto increment [would overwrite a previously set record.]
        if ( ( fieldname.isEmpty() ) || ( fieldname == getGeometryColumn() ) || ( fieldname == getPrimaryKey() ) )
        {
          continue;
        }
        sql += separator + QgsSpatiaLiteUtils::quotedIdentifier( fieldname );
        values += separator + '?';
        separator = ',';
      }
      sql += values;
      sql += ')';
      // SQLite prepared statement
      ret = sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
      if ( ret == SQLITE_OK )
      {
        for ( QgsFeatureList::iterator feature = flist.begin(); feature != flist.end(); ++feature )
        {
          // looping on each feature to insert
          QgsAttributes attributevec = feature->attributes();
          // resetting Prepared Statement and bindings
          sqlite3_reset( stmt );
          sqlite3_clear_bindings( stmt );
          // initializing the column counter
          ia = 0;
          if ( !getGeometryColumn().isEmpty() )
          {
            // binding GEOMETRY to Prepared Statement
            if ( !feature->hasGeometry() )
            {
              sqlite3_bind_null( stmt, ++ia );
            }
            else
            {
              unsigned char *wkb = nullptr;
              int wkb_size;
              QByteArray featureWkb = feature->geometry().asWkb();
              QgsSpatiaLiteUtils::convertFromGeosWKB( reinterpret_cast<const unsigned char *>( featureWkb.constData() ), featureWkb.size(), &wkb, &wkb_size, getCoordDimensions() );
              if ( !wkb )
              {
                sqlite3_bind_null( stmt, ++ia );
              }
              else
              {
                sqlite3_bind_blob( stmt, ++ia, wkb, wkb_size, QgsSpatiaLiteUtils::deleteWkbBlob );
              }
            }
          }
          for ( int i = 0; i < attributevec.count(); ++i )
          {
            QVariant v = attributevec.at( i );
            // binding values for each attribute
            if ( i >= getAttributeFields().count() )
            {
              break;
            }
            QString fieldname = getAttributeFields().at( i ).name();
            // do not write the Primary Key, will be set with auto increment [would overwrite a previously set record.]
            if ( ( fieldname.isEmpty() ) || ( fieldname == getGeometryColumn() ) || ( fieldname == getPrimaryKey() ) )
            {
              continue;
            }
            QVariant::Type type = getAttributeFields().at( i ).type();
            if ( !v.isValid() )
            {
              ++ia;
            }
            else if ( v.isNull() )
            {
              // binding a NULL value
              sqlite3_bind_null( stmt, ++ia );
            }
            else if ( type == QVariant::Int )
            {
              // binding an INTEGER value
              sqlite3_bind_int( stmt, ++ia, v.toInt() );
            }
            else if ( type == QVariant::LongLong )
            {
              // binding a LONGLONG value
              sqlite3_bind_int64( stmt, ++ia, v.toLongLong() );
            }
            else if ( type == QVariant::Double )
            {
              // binding a DOUBLE value
              sqlite3_bind_double( stmt, ++ia, v.toDouble() );
            }
            else if ( type == QVariant::String )
            {
              QString stringVal = v.toString();
              // binding a TEXT value
              QByteArray ba = stringVal.toUtf8();
              sqlite3_bind_text( stmt, ++ia, ba.constData(), ba.size(), SQLITE_TRANSIENT );
            }
            else if ( type == QVariant::StringList || type == QVariant::List )
            {
              const QByteArray ba = QgsJsonUtils::encodeValue( v ).toUtf8();
              sqlite3_bind_text( stmt, ++ia, ba.constData(), ba.size(), SQLITE_TRANSIENT );
            }
            else if ( type == QVariant::QVariant::Date || type == QVariant::DateTime )
            {
              QString stringVal = v.toString();
              const QByteArray ba = stringVal.toUtf8();
              sqlite3_bind_text( stmt, ++ia, ba.constData(), ba.size(), SQLITE_TRANSIENT );
            }
            else
            {
              // Unknown type: bind a NULL value
              sqlite3_bind_null( stmt, ++ia );
            }
          }
          // performing actual row insert
          ret = sqlite3_step( stmt );
          if ( ret == SQLITE_DONE || ret == SQLITE_ROW )
          {
            // update feature id
            if ( !( flags & QgsFeatureSink::FastInsert ) )
            {
              feature->setId( sqlite3_last_insert_rowid( dbSqliteHandle() ) );
            }
            mNumberFeatures++; // Will be reset in getLayerExtent after UpdateLayerStatistics
          }
          else
          {
            // some unexpected error occurred
            const char *err = sqlite3_errmsg( dbSqliteHandle() );
            errMsg = ( char * ) sqlite3_malloc( ( int ) strlen( err ) + 1 );
            strcpy( errMsg, err );
            break;
          }
        }
        sqlite3_finalize( stmt );
        if ( ret == SQLITE_DONE || ret == SQLITE_ROW )
        {
          ret = sqlite3_exec( dbSqliteHandle(), "COMMIT", nullptr, nullptr, &errMsg );
        }
      } // prepared statement
    } // BEGIN statement
    if ( ret != SQLITE_OK )
    {
      errorMessage = QString( tr( "SQLite error: %2 rc=%3\nSQL: %1" ).arg( sql, errMsg ? errMsg : tr( "unknown cause" ) ).arg( ret ) );
      if ( errMsg )
      {
        sqlite3_free( errMsg );
      }
      if ( toCommit )
      {
        // ROLLBACK after some previous error
        ( void )sqlite3_exec( dbSqliteHandle(), "ROLLBACK", nullptr, nullptr, nullptr );
      }
    }
    else
    {
      getLayerExtent( true, true );
    }
  }
  return ret == SQLITE_OK;
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::deleteLayerFeatures
//-----------------------------------------------------------------
bool SpatialiteDbLayer::deleteLayerFeatures( const QgsFeatureIds &id, QString &errorMessage )
{
  sqlite3_stmt *stmt = nullptr;
  char *errMsg = nullptr;
  QString sql;
  bool bUpdateExtent = true;
  bool bUpdateStatistics = true;
  int ret = sqlite3_exec( dbSqliteHandle(), "BEGIN", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg );
    return false;
  }
  sql = QStringLiteral( "DELETE FROM %1 WHERE %2=?" ).arg( QgsSpatiaLiteUtils::quotedIdentifier( getTableName() ), QgsSpatiaLiteUtils::quotedIdentifier( getPrimaryKey() ) );
  // SQLite prepared statement
  if ( sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr ) != SQLITE_OK )
  {
    // some error occurred
    errorMessage = QString( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( dbSqliteHandle() ) ) );
    return false;
  }
  for ( QgsFeatureIds::const_iterator it = id.begin(); it != id.end(); ++it )
  {
    // looping on each feature to be deleted
    // resetting Prepared Statement and bindings
    sqlite3_reset( stmt );
    sqlite3_clear_bindings( stmt );
    qint64 fid = FID_TO_NUMBER( *it );
    sqlite3_bind_int64( stmt, 1, fid );
    // performing actual row deletion
    ret = sqlite3_step( stmt );
    if ( ret == SQLITE_DONE || ret == SQLITE_ROW )
    {
      mNumberFeatures--; // Will be reset in getLayerExtent after UpdateLayerStatistics
    }
    else
    {
      // some unexpected error occurred
      const char *err = sqlite3_errmsg( dbSqliteHandle() );
      errMsg = ( char * ) sqlite3_malloc( ( int ) strlen( err ) + 1 );
      strcpy( errMsg, err );
      handleError( sql, errMsg, true );
      return false;
    }
  }
  sqlite3_finalize( stmt );
  ret = sqlite3_exec( dbSqliteHandle(), "COMMIT", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, true );
    return false;
  }
  getLayerExtent( bUpdateExtent, bUpdateStatistics );
  return true;
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::truncateLayerTableRows
//-----------------------------------------------------------------
bool SpatialiteDbLayer::truncateLayerTableRows()
{
  char *errMsg = nullptr;
  QString sql;
  bool bUpdateExtent = true;
  bool bUpdateStatistics = true;
  int ret = sqlite3_exec( dbSqliteHandle(), "BEGIN", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg );
    return false;
  }
  sql = QStringLiteral( "DELETE FROM %1" ).arg( QgsSpatiaLiteUtils::quotedIdentifier( getTableName() ) );
  ret = sqlite3_exec( dbSqliteHandle(), sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, true );
    return false;
  }
  ret = sqlite3_exec( dbSqliteHandle(), "COMMIT", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, true );
    return false;
  }
  getLayerExtent( bUpdateExtent, bUpdateStatistics );
  return true;
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::changeLayerGeometryValues
//-----------------------------------------------------------------
bool SpatialiteDbLayer::changeLayerGeometryValues( const QgsGeometryMap &geometry_map )
{
  int ret = SQLITE_ABORT; // Container not supported
  if ( isLayerSpatialite() )
  {
    sqlite3_stmt *stmt = nullptr;
    char *errMsg = nullptr;
    QString sql;

    ret = sqlite3_exec( dbSqliteHandle(), "BEGIN", nullptr, nullptr, &errMsg );
    if ( ret != SQLITE_OK )
    {
      handleError( sql, errMsg );
      return false;
    }
    sql = QStringLiteral( "UPDATE %1 SET %2=GeomFromWKB(?, %3) WHERE %4=?" )
          .arg( QgsSpatiaLiteUtils::quotedIdentifier( getTableName() ), QgsSpatiaLiteUtils::quotedIdentifier( getGeometryColumn() ) )
          .arg( getSrid() )
          .arg( QgsSpatiaLiteUtils::quotedIdentifier( getPrimaryKey() ) );
    // SQLite prepared statement
    if ( sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr ) != SQLITE_OK )
    {
      // some error occurred
      QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( dbSqliteHandle() ) ), tr( "SpatiaLite" ) );
      return false;
    }
    for ( QgsGeometryMap::const_iterator iter = geometry_map.constBegin(); iter != geometry_map.constEnd(); ++iter )
    {
      // resetting Prepared Statement and bindings
      sqlite3_reset( stmt );
      sqlite3_clear_bindings( stmt );
      // binding GEOMETRY to Prepared Statement
      unsigned char *wkb = nullptr;
      int wkb_size;
      QByteArray iterWkb = iter->asWkb();
      QgsSpatiaLiteUtils::convertFromGeosWKB( reinterpret_cast<const unsigned char *>( iterWkb.constData() ), iterWkb.size(), &wkb, &wkb_size, getCoordDimensions() );
      if ( !wkb )
      {
        sqlite3_bind_null( stmt, 1 );
      }
      else
      {
        sqlite3_bind_blob( stmt, 1, wkb, wkb_size, QgsSpatiaLiteUtils::deleteWkbBlob );
      }
      sqlite3_bind_int64( stmt, 2, FID_TO_NUMBER( iter.key() ) );
      // performing actual row update
      ret = sqlite3_step( stmt );
      if ( ret == SQLITE_DONE || ret == SQLITE_ROW )
        ;
      else
      {
        // some unexpected error occurred
        const char *err = sqlite3_errmsg( dbSqliteHandle() );
        errMsg = ( char * ) sqlite3_malloc( ( int ) strlen( err ) + 1 );
        strcpy( errMsg, err );
        handleError( sql, errMsg, true );
        return false;
      }
    }
    sqlite3_finalize( stmt );
    ret = sqlite3_exec( dbSqliteHandle(), "COMMIT", nullptr, nullptr, &errMsg );
    if ( ret != SQLITE_OK )
    {
      handleError( sql, errMsg, true );
      return false;
    }
  }
  return ret == SQLITE_OK;
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::addLayerAttributes
//-----------------------------------------------------------------
bool SpatialiteDbLayer::addLayerAttributes( const QList<QgsField> &attributes )
{
  char *errMsg = nullptr;
  QString sql;
  if ( attributes.isEmpty() )
  {
    return true;
  }
  int ret = sqlite3_exec( dbSqliteHandle(), "BEGIN", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg );
    return false;
  }
  for ( QList<QgsField>::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter )
  {
    sql = QStringLiteral( "ALTER TABLE \"%1\" ADD COLUMN \"%2\" %3" )
          .arg( getTableName(), iter->name(), iter->typeName() );
    ret = sqlite3_exec( dbSqliteHandle(), sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
    if ( ret != SQLITE_OK )
    {
      handleError( sql, errMsg, true );
      return false;
    }
  }
  ret = sqlite3_exec( dbSqliteHandle(), "COMMIT", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, true );
    return false;
  }
  getNumberFeatures( true );
  // reload columns
  getCapabilities( true );
  return true;
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::changeLayerAttributeValues
//-----------------------------------------------------------------
bool SpatialiteDbLayer::changeLayerAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  char *errMsg = nullptr;
  QString sql;
  if ( attr_map.isEmpty() )
  {
    return true;
  }
  int ret = sqlite3_exec( dbSqliteHandle(), "BEGIN", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg );
    return false;
  }
  for ( QgsChangedAttributesMap::const_iterator iter = attr_map.begin(); iter != attr_map.end(); ++iter )
  {
    // Loop over all changed features
    QgsFeatureId fid = iter.key();
    // skip added features
    if ( FID_IS_NEW( fid ) )
    {
      continue;
    }
    const QgsAttributeMap &attrs = iter.value();
    if ( attrs.isEmpty() )
    {
      continue;
    }
    QString sql = QStringLiteral( "UPDATE %1 SET " ).arg( QgsSpatiaLiteUtils::quotedIdentifier( getTableName() ) );
    bool first = true;
    // cycle through the changed attributes of the feature
    for ( QgsAttributeMap::const_iterator siter = attrs.begin(); siter != attrs.end(); ++siter )
    {
      // Loop over all changed attributes
      try
      {
        QgsField fld = getAttributeField( siter.key() );
        const QVariant &val = siter.value();
        if ( !first )
        {
          sql += ',';
        }
        else
        {
          first = false;
        }
        QVariant::Type type = fld.type();
        if ( val.isNull() || !val.isValid() )
        {
          // binding a NULL value
          sql += QStringLiteral( "%1=NULL" ).arg( QgsSpatiaLiteUtils::quotedIdentifier( fld.name() ) );
        }
        else if ( type == QVariant::Int || type == QVariant::LongLong || type == QVariant::Double )
        {
          // binding a NUMERIC value
          sql += QStringLiteral( "%1=%2" ).arg( QgsSpatiaLiteUtils::quotedIdentifier( fld.name() ), val.toString() );
        }
        else if ( type == QVariant::StringList || type == QVariant::List )
        {
          // binding an array value
          sql += QStringLiteral( "%1=%2" ).arg( QgsSpatiaLiteUtils::quotedIdentifier( fld.name() ), QgsSpatiaLiteUtils::quotedValue( QgsJsonUtils::encodeValue( val ) ) );
        }
        else
        {
          // binding a TEXT value
          sql += QStringLiteral( "%1=%2" ).arg( QgsSpatiaLiteUtils::quotedIdentifier( fld.name() ), QgsSpatiaLiteUtils::quotedValue( val.toString() ) );
        }
      }
      catch ( QgsSpatiaLiteUtils::SLFieldNotFound )
      {
        // Field was missing - shouldn't happen
      }
    }
    sql += QStringLiteral( " WHERE %1=%2" ).arg( QgsSpatiaLiteUtils::quotedIdentifier( getPrimaryKey() ) ).arg( fid );
    ret = sqlite3_exec( dbSqliteHandle(), sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
    if ( ret != SQLITE_OK )
    {
      handleError( sql, errMsg, true );
      return false;
    }
  }
  ret = sqlite3_exec( dbSqliteHandle(), "COMMIT", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, true );
    return false;
  }
  return true;
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::getAttributeField
//-----------------------------------------------------------------
QgsField SpatialiteDbLayer::getAttributeField( int index ) const
{
  if ( index < 0 || index >= getAttributeFields().count() )
  {
    QgsMessageLog::logMessage( tr( "FAILURE: Field %1 not found." ).arg( index ), tr( "SpatiaLite" ) );
    throw QgsSpatiaLiteUtils::SLFieldNotFound();
  }
  return getAttributeFields().at( index );
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::createLayerAttributeIndex
//-----------------------------------------------------------------
bool SpatialiteDbLayer::createLayerAttributeIndex( int field )
{
  int ret = SQLITE_ABORT; // Container not supported
  if ( isLayerSpatialite() )
  {
    char *errMsg = nullptr;
    if ( field < 0 || field >= getAttributeFields().count() )
    {
      return false;
    }
    QString sql;
    QString fieldName;
    ret = sqlite3_exec( dbSqliteHandle(), "BEGIN", nullptr, nullptr, &errMsg );
    if ( ret != SQLITE_OK )
    {
      handleError( sql, errMsg );
      return false;
    }
    fieldName = getAttributeFields().at( field ).name();
    sql = QStringLiteral( "CREATE INDEX IF NOT EXISTS %1 ON \"%2\" (%3)" )
          .arg( QgsSpatiaLiteUtils::createIndexName( getTableName(), fieldName ),
                getTableName(), QgsSpatiaLiteUtils::quotedIdentifier( fieldName ) );
    ret = sqlite3_exec( dbSqliteHandle(), sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
    if ( ret != SQLITE_OK )
    {
      handleError( sql, errMsg, true );
      return false;
    }
    ret = sqlite3_exec( dbSqliteHandle(), "COMMIT", nullptr, nullptr, &errMsg );
    if ( ret != SQLITE_OK )
    {
      handleError( sql, errMsg, true );
      return false;
    }
  }
  return ret == SQLITE_OK;
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::geomParam
//-----------------------------------------------------------------
QString SpatialiteDbLayer::geomParam() const
{
  QString geometry;
  bool forceMulti = QgsWkbTypes::isMultiType( getGeometryType() );
  // ST_Multi function is available from QGIS >= 2.4
  bool hasMultiFunction = dbSpatialiteVersionMajor() > 2 ||
                          ( dbSpatialiteVersionMajor() == 2 && dbSpatialiteVersionMinor() >= 4 );

  if ( forceMulti && hasMultiFunction )
  {
    geometry += QLatin1String( "ST_Multi(" );
  }
  geometry += QStringLiteral( "GeomFromWKB(?, %2)" ).arg( mSrid );
  if ( forceMulti && hasMultiFunction )
  {
    geometry += ')';
  }
  return geometry;
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::handleError
//-----------------------------------------------------------------
void SpatialiteDbLayer::handleError( const QString &sql, char *errorMessage, bool rollback )
{
  QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, errorMessage ? errorMessage : tr( "unknown cause" ) ), tr( "SpatiaLite" ) );
  // unexpected error
  if ( errorMessage )
  {
    sqlite3_free( errorMessage );
  }
  if ( rollback )
  {
    // ROLLBACK after some previous error
    ( void )sqlite3_exec( dbSqliteHandle(), "ROLLBACK", nullptr, nullptr, nullptr );
  }
}
//-----------------------------------------------------------------
// floatToByteArray
//-----------------------------------------------------------------
unsigned char *floatToByteArray( float f )
{
  unsigned char *ret = ( unsigned char * )malloc( 4 * sizeof( unsigned char ) );
  unsigned int asInt = *( ( int * )&f );
  int i;
  for ( i = 0; i < 4; i++ )
  {
    ret[i] = ( asInt >> 8 * i ) & 0xFF;
  }
  return ret;
// -118.625 [1] [10000101] [11011010100000000000000]
// 11000010111011010100000000000000
// 1 sign bit | 8 exponent bit | 23 fraction bits
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::getMapBandsFromRasterLite2
//-----------------------------------------------------------------
QList<QByteArray *> SpatialiteDbLayer::getMapBandsFromRasterLite2( int width, int height, const QgsRectangle &viewExtent, QString styleName, QString bgColor, QString &errCause )
{
  bool bTransparent = true;
  int quality = 80;
  bool bReaspect = true;
  QList<QByteArray *> imageBands;
  int iQImageBands = 0;
  int iLayerBands = getLayerNumBands();
  int iPixelType = getLayerRasterPixelType();
  Qgis::DataType dataType = getLayerRasterDataType();
  QString mimeType = QStringLiteral( "image/png" );
  switch ( iPixelType )
  {
    case 17: // MONOCHROME
    case 18: // PALETTE
    case 19: // GRAYSCALE
    case 20: // RGB
      mimeType = QStringLiteral( "image/png" );
      break;
    case 21: // MULTIBAND [ > 1 < 256]
    case 22: // DATAGRID [ 1 band only]
      mimeType = QStringLiteral( "image/tiff" );
      break;
  }
  // mimeType = QStringLiteral( "image/png" );
  QImage imageResult = rl2GetMapImageFromRaster( getSrid(), width, height, viewExtent, errCause, styleName, mimeType, bgColor, bTransparent, quality, bReaspect );
  QgsDebugMsgLevel( QString( "SpatialiteDbLayer::getMapBandsFromRasterLite2[%5] mimeType[%8] Pixel/DataType[%9,%10] bitPlaneCount[%1] depth[%2]  format[%3] byteCount[%4] isGrayscale[%6] allGray[%7] Bands[%11]" ).arg( imageResult.bitPlaneCount() ).arg( imageResult.depth() ).arg( imageResult.format() ).arg( imageResult.byteCount() ).arg( getCoverageName() ).arg( imageResult.isGrayscale() ).arg( imageResult.allGray() ).arg( mimeType ).arg( iPixelType ).arg( ( int )dataType ).arg( iLayerBands ), 4 );
  if ( imageResult.byteCount() > 0 )
  {
    int iBandPosition = -1;
    // What we are receiving from RasterLite2 in the form of a PNG
    iQImageBands = imageResult.bitPlaneCount() / 8;
    int iBytesPerBand = imageResult.byteCount() / iQImageBands;
    if ( ( iPixelType == 21 ) || ( iPixelType == 22 ) )
    {
      // What we are receiving from RasterLite2 in the form of a TIF
      iBytesPerBand = imageResult.byteCount() / iLayerBands;
    }
    // Note: We will not save the Alpha-Band or Band 2 and 3 for MONOCHROME, PALETTE, GRAYSCALE images.
    imageBands.reserve( iLayerBands );  // What we are returning to QgsRasterLite2Provider
    for ( int i = 0; i <  iLayerBands; i++ )
    {
      QByteArray *imageBand = new QByteArray();
      imageBand ->reserve( iBytesPerBand );
      imageBands.append( imageBand );
    }
    if ( ( iPixelType == 21 ) || ( iPixelType == 22 ) )
    {
      // MULTIBAND and DATAGRID
      iBandPosition = 0;
      const QImage *pImageResult = &imageResult; // [avoid making a deep copy]
      const uchar *pBits = pImageResult->bits();
      QRgb *bits = reinterpret_cast< QRgb * >( ( unsigned int * )pImageResult->bits() );
      char float_data[sizeof( float )];
      QString sRGB;
      for ( int j = 0; j < ( imageResult.byteCount() / iQImageBands ); j++ )
      {
        for ( int iBand = 0; iBand < iQImageBands; iBand++ )
        {
          switch ( iBand )
          {
            case 0:
              float_data[iBand ] = ( bits[j] >> 24 );
              break;
            case 1:
              float_data[iBand ] = ( bits[j] >> 16 );
              break;
            case 2:
              float_data[iBand ] = ( bits[j] >> 8 );
              break;
            case 3:
              float_data[iBand ] = ( bits[j] );
              break;
          }
        }
        float height_retrieve = 0;
        memcpy( &height_retrieve, float_data, sizeof height_retrieve );  // bytes to float
        sRGB = QString( "Alpha(%1) Red(%2)  Green(%3) Blue(%4)" ).arg( QString::number( float_data[0], 16 ) ).arg( QString::number( float_data[1], 16 ) ).arg( QString::number( float_data[2], 16 ) ).arg( QString::number( float_data[3], 16 ) );
        QgsDebugMsgLevel( QString( "height_retrieve: %1: %2" ).arg( height_retrieve ).arg( sRGB ), 5 );
      }
      sRGB = QString( "Alpha(%1) Red(%2)  Green(%3) Blue(%4)" ).arg( QString::number( qAlpha( bits[0] ), 16 ) ).arg( QString::number( qRed( bits[0] ), 16 ) ).arg( QString::number( qGreen( bits[0] ), 16 ) ).arg( QString::number( qBlue( bits[0] ), 16 ) );
      QgsDebugMsgLevel( QString( "input: %1: %2" ).arg( ( float )bits[0] ).arg( sRGB ), 5 );
      float height_pixel = 33.160;
      QRgb iheight_pixel = ( QRgb )( ( uint )height_pixel );
      sRGB = QString( "Alpha(%1) Red(%2)  Green(%3) Blue(%4)" ).arg( qAlpha( ( uint )height_pixel ) ).arg( qRed( ( uint )height_pixel ) ).arg( qGreen( ( uint )height_pixel ) ).arg( qBlue( ( uint )height_pixel ) );
      QgsDebugMsgLevel( QString( "sample: %1: %2" ).arg( ( float )iheight_pixel ).arg( sRGB ), 5 );
      unsigned char *asBytes = floatToByteArray( height_pixel );
      sRGB = QString( "Alpha(%1) Red(%2)  Green(%3) Blue(%4)" ).arg( QString::number( asBytes[0], 16 ) ).arg( QString::number( asBytes[1], 16 ) ).arg( QString::number( asBytes[2], 16 ) ).arg( QString::number( asBytes[3], 16 ) );
      QgsDebugMsgLevel( QString( "float_bytes: %1: %2" ).arg( ( float )iheight_pixel ).arg( sRGB ), 5 );
      memcpy( float_data, &height_pixel, sizeof height_pixel ); // float to bytes
      float height_retrieve = 0;
      memcpy( &height_retrieve, float_data, sizeof height_retrieve );  // bytes to float
      sRGB = QString( "Alpha(%1) Red(%2)  Green(%3) Blue(%4)" ).arg( QString::number( float_data[0], 16 ) ).arg( QString::number( float_data[1], 16 ) ).arg( QString::number( float_data[2], 16 ) ).arg( QString::number( float_data[3], 16 ) );
      QgsDebugMsgLevel( QString( "height_retrieve: %1: %2" ).arg( height_retrieve ).arg( sRGB ), 5 );
      for ( int iBand = 0; iBand < iLayerBands; iBand++ )
      {
        // 33.62 .48967e+9: Alpha(208) Red(0)  Green(45) Blue(64)
        imageBands.at( iBand )->append( ( const char * )pBits, iBytesPerBand );
        QgsDebugMsgLevel( QString( "SpatialiteDbLayer::getMapBandsFromRasterLite2[%5] mimeType[%8] Pixel/DataType[%9,%10] bitPlaneCount[%1] depth[%2]  format[%3] byteCount[%4] isGrayscale[%6] allGray[%7] Band[%11] BytesPerBand[%12] BandPosition[%13] copied_size[%14]" ).arg( imageResult.bitPlaneCount() ).arg( imageResult.depth() ).arg( imageResult.format() ).arg( imageResult.byteCount() ).arg( getCoverageName() ).arg( imageResult.isGrayscale() ).arg( imageResult.allGray() ).arg( mimeType ).arg( iPixelType ).arg( ( int )dataType ).arg( iBand ).arg( iBytesPerBand ).arg( iBandPosition ).arg( imageBands.at( iBand )->size() ), 5 );
        pBits += iBytesPerBand;
        iBandPosition += iBytesPerBand;
      }
    }
    else
    {
      QgsDebugMsgLevel( QString( "SpatialiteDbLayer::getMapBandsFromRasterLite2[%5] mimeType[%8] Pixel/DataType[%9,%10] bitPlaneCount[%1] depth[%2]  format[%3] byteCount[%4] isGrayscale[%6] allGray[%7]" ).arg( imageResult.bitPlaneCount() ).arg( imageResult.depth() ).arg( imageResult.format() ).arg( imageResult.byteCount() ).arg( getCoverageName() ).arg( imageResult.isGrayscale() ).arg( imageResult.allGray() ).arg( mimeType ).arg( iPixelType ).arg( ( int )dataType ), 5 );
      // MONOCHROME, PALETTE, GRAYSCALE, RGB
      // We will copy what we need into the QByteArray and let QImage discard the original Data
      const QImage *pImageResult = &imageResult; // [avoid making a deep copy]
      QRgb *bits = reinterpret_cast< QRgb * >( ( unsigned int * )pImageResult->bits() );
      if ( bits )
      {
        for ( int j = 0; j < ( imageResult.byteCount() / iQImageBands ); j++ )
        {
          iBandPosition++;
          unsigned char alpha = 0;
#if 0
          if ( j < 10 )
          {
            QString sRGB = QString( "Alpha(%1) Red(%2)  Green(%3) Blue(%4)" ).arg( qAlpha( bits[j] ) ).arg( qRed( bits[j] ) ).arg( qGreen( bits[j] ) ).arg( qBlue( bits[j] ) );
            QgsDebugMsgLevel( QString( "%1: %2" ).arg( j ).arg( sRGB ), 5 );
          }
#endif
          for ( int iBand = 0; iBand < iQImageBands; iBand++ )
          {
            unsigned char value = 255;
            if ( iBand > 0 )
            {
              // Retrieve the nodata value for this Band
              value = ( unsigned char )mLayerBandsNodata.value( ( iBand - 1 ) );
            }
            // Note: convert RGB to Grayscale: (r*11+g*16+b*5)/32
            switch ( iBand )
            {
              case 0:
                value = ( bits[j] >> 24 );
                alpha = value;
                break;
              case 1:
                if ( alpha != 0 )
                {
                  // Not a nodata pixel
                  value = ( bits[j] >> 16 );
                }
                break;
              case 2:
                if ( alpha != 0 )
                {
                  // Not a nodata pixel
                  value = ( bits[j] >> 8 );
                }
                break;
              case 3:
                if ( alpha != 0 )
                {
                  // Not a nodata pixel
                  value = ( bits[j] );
                }
                break;
            }
            if ( ( iBand > 0 ) && ( ( iBand - 1 ) < iLayerBands ) )
            {
              // Store only those values needed for QGis, ignoring everything else
              imageBands.at( ( iBand - 1 ) )->append( value );
            }
          }
        }
      }
    }
  }
  return imageBands;
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::getMapImageFromRasterLite2
//-----------------------------------------------------------------
QImage SpatialiteDbLayer::getMapImageFromRasterLite2( int width, int height, const QgsRectangle &viewExtent, QString styleName, QString mimeType, QString bgColor, QString &errCause )
{
  bool bTransparent = true;
  int quality = 80;
  bool bReaspect = true;
  return rl2GetMapImageFromRaster( getSrid(), width, height, viewExtent, errCause, styleName, mimeType, bgColor, bTransparent, quality, bReaspect );
}
//-----------------------------------------------------------------
// SpatialiteDbLayer::rl2GetMapImageFromRaster
//-----------------------------------------------------------------
QImage SpatialiteDbLayer::rl2GetMapImageFromRaster( int destSrid, int width, int height, const QgsRectangle &viewExtent, QString &errCause,
    QString styleName, QString mimeType, QString bgColor, bool bTransparent, int quality, bool bReaspect )
{
  QImage imageResult = QImage( width, height, QImage::Format_ARGB32 );
  QString sExtension = "png";
  // Insure that a RasterLite2 connection exists and that the ViewExtent, Image width/height are valid
  if ( ( layerHasRasterlite2() ) && ( ( !viewExtent.isEmpty() ) && ( width > 0 ) && ( height > 0 ) ) )
  {
    // further sanity checks
    if ( styleName.isEmpty() )
    {
      styleName =  getLayerStyleSelected();
    }
    if ( styleName.isEmpty() )
    {
      styleName = "default";
    }
    // TODO remove this - only for testing
    styleName = "default";
    if ( ( mimeType.contains( "image/pdf" ) ) || ( mimeType.contains( "iapplication/x-pdf'" ) ) )
    {
      // these formats are not supported by QImage
      mimeType = "image/png";
    }
    if ( mimeType.contains( "image/tif" ) )
    {
      // correct a typo
      mimeType = "image/tiff";
    }
    if ( mimeType.contains( "image/jpg" ) )
    {
      // correct a typo
      mimeType = "image/jpeg";
    }
    // Note: 'image/jpeg' should be avoided due to lack of transparency support
    if ( mimeType.isEmpty() )
    {
      mimeType = "image/png";
    }
    mimeType = "image/png";
    if ( bgColor.isEmpty() )
    {
      bgColor = "#ffffff";
    }
    if ( ( quality < 0 ) || ( quality > 100 ) )
    {
      quality = 0;
      if ( mimeType.contains( "jpeg" ) )
      {
        quality = 80;
      }
    }
    if ( mimeType.contains( "image/tif" ) )
    {
      sExtension = "tif";
    }
    if ( mimeType.contains( "image/jp" ) )
    {
      sExtension = "jpg";
    }
    // The bTransparent/quality values will be ignored if the mimi-type does not support it ('image/jpeg')
    // The bgColor value will be ignored if the mimi-type support transparity and bTransparent=true ('image/png')
    sqlite3_stmt *stmt = nullptr;
    QString sBuildMBR = QString();
    if ( getSrid()  != destSrid )
    {
      sBuildMBR += QString( "ST_Transform(" );
    }
    // QgsRasterBlock::printValue: double value with all necessary significant digits.
    //  It is ensured that conversion back to double gives the same number.
    sBuildMBR += QStringLiteral( "BuildMbr(%1,%2,%3,%4,%5)" )
                 .arg( QgsRasterBlock::printValue( viewExtent.xMinimum() ),
                       QgsRasterBlock::printValue( viewExtent.yMinimum() ),
                       QgsRasterBlock::printValue( viewExtent.xMaximum() ),
                       QgsRasterBlock::printValue( viewExtent.yMaximum() ) ).arg( getSrid() );
    if ( getSrid() != destSrid )
    {
      sBuildMBR += QString( ",%1)," ).arg( getSrid() );
    }
    QString sql = QString( "SELECT RL2_GetMapImageFromRaster('main','%1',%2," ).arg( getCoverageName() ).arg( sBuildMBR );
    sql += QString( "%1,%2," ).arg( width ).arg( height );
    sql += QString( "'%1','%2','%3'," ).arg( styleName ).arg( mimeType ).arg( bgColor );
    sql += QString( "%1,%2,%3)" ).arg( ( int )bTransparent ).arg( quality ).arg( ( int )bReaspect );
    QgsDebugMsgLevel( QString( "sql[%1]" ).arg( sql ), 5 );
    if ( sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr ) == SQLITE_OK )
    {
      //----------------------------------------------------------------
      // libpng warning: [gdal, read] Application was compiled with png.h from libpng-1.6.10
      // libpng warning: [gdal, read] Application  is  running with png.c from libpng-1.2.56
      // libpng error: [gdal, read] Incompatible libpng version in application and library
      // Until libpng 1.4.21 contatains faulty 'if' condition (should only effect version < 0.90)
      //----------------------------------------------------------------
      // If gdal is compiled with --with-png=internal
      // this could fail, since gdal is also being loaded and
      // it seem that its version of libpng 1.2.56 is used.
      // https://trac.osgeo.org/gdal/ticket/7023#ticket
      //----------------------------------------------------------------
      int i_count_bytes = 0;
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
        {
          const uchar *p_blob = ( const uchar * )sqlite3_column_blob( stmt, 0 );
          i_count_bytes = sqlite3_column_bytes( stmt, 0 );
          // the png/jpeg/tiff data will be used to create a ARGB32, for which memory has been reserved in QImage
          imageResult.loadFromData( p_blob, i_count_bytes );
        }
      }
      sqlite3_finalize( stmt );
      if ( i_count_bytes > 0 )
      {
        imageResult.save( QString( "%1/%2.%3_%4.%5" ).arg( getDatabaseDirectoryName() ).arg( getCoverageName() ).arg( width ).arg( height ).arg( sExtension ) );
        return imageResult;
      }
      else
      {
        errCause = QString( "rl2GetMapImageFromRaster: image[%1 x %2] for area[%3] returned %4 Bytes. sql[%5]" ).arg( width ).arg( height ).arg( sBuildMBR ).arg( i_count_bytes ).arg( sql );
      }
    }
  }
  return imageResult;
}
//-----------------------------------------------------------------
// Class QgsSpatiaLiteUtils
//-----------------------------------------------------------------
// QgsSpatiaLiteUtils::CreateSpatialiteDbInfo
//-----------------------------------------------------------------
SpatialiteDbInfo *QgsSpatiaLiteUtils::CreateSpatialiteDbInfo( QString dbPath,  bool bLoadLayers,  bool bShared, SpatialiteDbInfo::SpatialMetadata dbCreateOption )
{
  SpatialiteDbInfo *spatialiteDbInfo = nullptr;
  QString sLayerName = QString::null;
  QgsSqliteHandle *qSqliteHandle = QgsSqliteHandle::openDb( dbPath, bShared, sLayerName, bLoadLayers, dbCreateOption );
  if ( ( qSqliteHandle ) && ( qSqliteHandle->getSpatialiteDbInfo() ) )
  {
    spatialiteDbInfo = qSqliteHandle->getSpatialiteDbInfo();
    if ( ( spatialiteDbInfo ) && ( spatialiteDbInfo->isDbSqlite3() ) )
    {
      if ( ( !spatialiteDbInfo->isDbSpatialite() ) && ( !spatialiteDbInfo->isDbGdalOgr() ) )
      {
        // The read Sqlite3 Container is not supported by QgsSpatiaLiteProvider, QRasterLite2Provider,QgsOgrProvider or QgsGdalProvider.
      }
      else
      {
        if ( ( bLoadLayers ) && ( spatialiteDbInfo->getSniffType() != SpatialiteDbInfo::SniffLoadLayers ) )
        {
          // SpatialiteDbInfo may have been created with SniffMinimal, but now we want all LayerInformation
          // -will read all Layers now, instead of 1 by 1 which would be slower [by large Databases noticeable]
          spatialiteDbInfo->setSniffType( SpatialiteDbInfo::SniffLoadLayers );
        }
      }
    }
    else
    {
      qSqliteHandle->invalidate(); // Not a Database
      if ( spatialiteDbInfo )
      {
        if ( spatialiteDbInfo->getFileMimeType() == SpatialiteDbInfo::MimeNotExists )
        {
          // File does not exist
          delete spatialiteDbInfo;
          spatialiteDbInfo = nullptr;
        }
        // Allow the caller to 'sniff' the result of what it may be.
      }
    }
  }
  return spatialiteDbInfo;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteUtils::createSpatialDatabase
//-----------------------------------------------------------------
bool QgsSpatiaLiteUtils::createSpatialDatabase( QString sDatabaseFileName, QString &errCause, SpatialiteDbInfo::SpatialMetadata dbCreateOption )
{
  bool bRc = false;
  bool bLoadLayers = false;
  bool bShared = false;
  QString sDatabaseType;
  //---------------------------------------------------------------
  QString sLayerName = QString::null;
  QgsSqliteHandle *qSqliteHandle = QgsSqliteHandle::openDb( sDatabaseFileName, bShared, sLayerName, bLoadLayers, dbCreateOption );
  if ( ( qSqliteHandle ) && ( qSqliteHandle->getSpatialiteDbInfo() ) )
  {
    SpatialiteDbInfo *spatialiteDbInfo = qSqliteHandle->getSpatialiteDbInfo();
    if ( ( spatialiteDbInfo ) && ( spatialiteDbInfo->isDbSqlite3() ) )
    {
      switch ( dbCreateOption )
      {
        case SpatialiteDbInfo::Spatialite40:
        case SpatialiteDbInfo::Spatialite45:
          sDatabaseType = "Spatialite";
          switch ( spatialiteDbInfo->dbSpatialMetadata() )
          {
            case SpatialiteDbInfo::SpatialiteLegacy:
            case SpatialiteDbInfo::Spatialite40:
            case SpatialiteDbInfo::Spatialite45:
              // this is a Database that can be used for QgsSpatiaLiteProvider
              bRc = true;
              break;
            default:
              break;
          }
          break;
        case SpatialiteDbInfo::SpatialiteGpkg:
          sDatabaseType = "GeoPackage";
          switch ( spatialiteDbInfo->dbSpatialMetadata() )
          {
            case SpatialiteDbInfo::SpatialiteGpkg:
              // this is a Database that can be used for QgsOgrProvider or QgsGdalProvider
              bRc = true;
              break;
            default:
              break;
          }
          break;
        case SpatialiteDbInfo::SpatialiteMBTiles:
          sDatabaseType = "MBTiles";
          switch ( spatialiteDbInfo->dbSpatialMetadata() )
          {
            case SpatialiteDbInfo::SpatialiteMBTiles:
              // this is a Database that can be used for QgsGdalProvider
              bRc = true;
              break;
            default:
              break;
          }
          break;
        default:
          break;
      }
      if ( !bRc )
      {
        errCause = QObject::tr( "The created Sqlite3-Container is not the expected Type: %1, is %2\n%3" ).arg( sDatabaseType ).arg( spatialiteDbInfo->dbSpatialMetadataString() ).arg( sDatabaseFileName );
      }
      delete spatialiteDbInfo;
      spatialiteDbInfo = nullptr;
    }
    else
    {
      errCause = QObject::tr( "Could not create a new database\n result is not a Sqlite3-Container.\n%1" ).arg( sDatabaseFileName );
    }
  }
  else
  {
    errCause = QObject::tr( "Could not create a new database\n cause unknown. \n%1" ).arg( sDatabaseFileName );
  }
  return bRc;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteUtils::createIndexName
//-----------------------------------------------------------------
QString QgsSpatiaLiteUtils::createIndexName( QString tableName, QString field )
{
  QRegularExpression safeExp( QStringLiteral( "[^a-zA-Z0-9]" ) );
  tableName.replace( safeExp, QStringLiteral( "_" ) );
  field.replace( safeExp, QStringLiteral( "_" ) );
  return QStringLiteral( "%1_%2_idx" ).arg( tableName, field );
}
//-----------------------------------------------------------------
// QgsSpatiaLiteUtils::quotedIdentifier
//-----------------------------------------------------------------
QString QgsSpatiaLiteUtils::quotedIdentifier( QString id )
{
  id.replace( '\"', QLatin1String( "\"\"" ) );
  return id.prepend( '\"' ).append( '\"' );
}
//-----------------------------------------------------------------
// QgsSpatiaLiteUtils::quotedValue
//-----------------------------------------------------------------
QString QgsSpatiaLiteUtils::quotedValue( QString value )
{
  if ( value.isNull() )
  {
    return QStringLiteral( "NULL" );
  }
  value.replace( '\'', QLatin1String( "''" ) );
  return value.prepend( '\'' ).append( '\'' );
}
//-----------------------------------------------------------------
// QgsSpatiaLiteUtils::deleteWkbBlob
//-----------------------------------------------------------------
void QgsSpatiaLiteUtils::deleteWkbBlob( void *wkbBlob )
{
  delete[]( char * )wkbBlob;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteUtils::GetSpatialiteLayerInfoWrapper
//-----------------------------------------------------------------
SpatialiteDbInfo *QgsSpatiaLiteUtils::GetSpatialiteLayerInfoWrapper( QString sDatabaseFileName, QString sLayerName, bool bLoadLayers, sqlite3 *sqlite_handle )
{
  SpatialiteDbInfo *spatialiteDbInfo = nullptr;
  if ( ( sqlite_handle ) && ( QFile::exists( sDatabaseFileName ) ) )
  {
    spatialiteDbInfo = new SpatialiteDbInfo( sDatabaseFileName, sqlite_handle );
    if ( ( spatialiteDbInfo ) && ( spatialiteDbInfo->isDbValid() ) )
    {
      // retrieve all possible Layers if sLayerName is empty or Null
      if ( spatialiteDbInfo->getSpatialiteLayerInfo( sLayerName, bLoadLayers ) )
      {
        return spatialiteDbInfo;
      }
    }
  }
  return spatialiteDbInfo;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteUtils::convertToGeosWKB
//-----------------------------------------------------------------
void QgsSpatiaLiteUtils::convertToGeosWKB( const unsigned char *blob,
    int blob_size,
    unsigned char **wkb,
    int *geom_size )
{
// attempting to convert to 2D/3D GEOS own WKB
  int type;
  int dims;
  int little_endian;
  int endian_arch = gaiaEndianArch();
  int entities;
  int rings;
  int points;
  int ie;
  int ib;
  int iv;
  int gsize = 6;
  const unsigned char *p_in;
  unsigned char *p_out;
  double coord;

  *wkb = nullptr;
  *geom_size = 0;
  if ( blob_size < 5 )
    return;
  if ( *( blob + 0 ) == 0x01 )
    little_endian = GAIA_LITTLE_ENDIAN;
  else
    little_endian = GAIA_BIG_ENDIAN;
  type = gaiaImport32( blob + 1, little_endian, endian_arch );
  if ( type == GAIA_POINTZ || type == GAIA_LINESTRINGZ
       || type == GAIA_POLYGONZ
       || type == GAIA_MULTIPOINTZ || type == GAIA_MULTILINESTRINGZ
       || type == GAIA_MULTIPOLYGONZ || type == GAIA_GEOMETRYCOLLECTIONZ )
    dims = 3;
  else if ( type == GAIA_POINTM || type == GAIA_LINESTRINGM
            || type == GAIA_POLYGONM || type == GAIA_MULTIPOINTM
            || type == GAIA_MULTILINESTRINGM || type == GAIA_MULTIPOLYGONM
            || type == GAIA_GEOMETRYCOLLECTIONM )
    dims = 3;
  else if ( type == GAIA_POINTZM || type == GAIA_LINESTRINGZM
            || type == GAIA_POLYGONZM || type == GAIA_MULTIPOINTZM
            || type == GAIA_MULTILINESTRINGZM || type == GAIA_MULTIPOLYGONZM
            || type == GAIA_GEOMETRYCOLLECTIONZM )
    dims = 4;
  else if ( type == GAIA_POINT || type == GAIA_LINESTRING
            || type == GAIA_POLYGON || type == GAIA_MULTIPOINT
            || type == GAIA_MULTILINESTRING || type == GAIA_MULTIPOLYGON
            || type == GAIA_GEOMETRYCOLLECTION )
    dims = 2;
  else
    return;

  if ( dims == 2 )
  {
    // already 2D: simply copying is required
    unsigned char *wkbGeom = new unsigned char[blob_size + 1];
    memcpy( wkbGeom, blob, blob_size );
    memset( wkbGeom + blob_size, 0, 1 );
    *wkb = wkbGeom;
    *geom_size = blob_size + 1;
    return;
  }

  // we need creating a 3D GEOS WKB
  p_in = blob + 5;
  switch ( type )
  {
    // compunting the required size
    case GAIA_POINTZ:
    case GAIA_POINTM:
    case GAIA_POINTZM:
      gsize += 3 * sizeof( double );
      break;
    case GAIA_LINESTRINGZ:
    case GAIA_LINESTRINGM:
    case GAIA_LINESTRINGZM:
      points = gaiaImport32( p_in, little_endian, endian_arch );
      gsize += 4;
      gsize += points * ( 3 * sizeof( double ) );
      break;
    case GAIA_POLYGONZ:
    case GAIA_POLYGONM:
      rings = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gsize += 4;
      for ( ib = 0; ib < rings; ib++ )
      {
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gsize += 4;
        gsize += points * ( 3 * sizeof( double ) );
        p_in += points * ( 3 * sizeof( double ) );
      }
      break;
    case GAIA_POLYGONZM:
      rings = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gsize += 4;
      for ( ib = 0; ib < rings; ib++ )
      {
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gsize += 4;
        gsize += points * ( 3 * sizeof( double ) );
        p_in += points * ( 4 * sizeof( double ) );
      }
      break;
    default:
      gsize += computeMultiWKB3Dsize( p_in, little_endian, endian_arch );
      break;
  }

  unsigned char *wkbGeom = new unsigned char[gsize];
  memset( wkbGeom, '\0', gsize );

// building GEOS 3D WKB
  *wkbGeom = 0x01;  // little endian byte order
  type = gaiaImport32( blob + 1, little_endian, endian_arch );
  switch ( type )
  {
    // setting Geometry TYPE
    case GAIA_POINTZ:
    case GAIA_POINTM:
    case GAIA_POINTZM:
      gaiaExport32( wkbGeom + 1, GEOS_3D_POINT, 1, endian_arch );
      break;
    case GAIA_LINESTRINGZ:
    case GAIA_LINESTRINGM:
    case GAIA_LINESTRINGZM:
      gaiaExport32( wkbGeom + 1, GEOS_3D_LINESTRING, 1, endian_arch );
      break;
    case GAIA_POLYGONZ:
    case GAIA_POLYGONM:
    case GAIA_POLYGONZM:
      gaiaExport32( wkbGeom + 1, GEOS_3D_POLYGON, 1, endian_arch );
      break;
    case GAIA_MULTIPOINTZ:
    case GAIA_MULTIPOINTM:
    case GAIA_MULTIPOINTZM:
      gaiaExport32( wkbGeom + 1, GEOS_3D_MULTIPOINT, 1, endian_arch );
      break;
    case GAIA_MULTILINESTRINGZ:
    case GAIA_MULTILINESTRINGM:
    case GAIA_MULTILINESTRINGZM:
      gaiaExport32( wkbGeom + 1, GEOS_3D_MULTILINESTRING, 1, endian_arch );
      break;
    case GAIA_MULTIPOLYGONZ:
    case GAIA_MULTIPOLYGONM:
    case GAIA_MULTIPOLYGONZM:
      gaiaExport32( wkbGeom + 1, GEOS_3D_MULTIPOLYGON, 1, endian_arch );
      break;
    case GAIA_GEOMETRYCOLLECTIONZ:
    case GAIA_GEOMETRYCOLLECTIONM:
    case GAIA_GEOMETRYCOLLECTIONZM:
      gaiaExport32( wkbGeom + 1, GEOS_3D_GEOMETRYCOLLECTION, 1, endian_arch );
      break;
  }
  p_in = blob + 5;
  p_out = wkbGeom + 5;
  switch ( type )
  {
    // setting Geometry values
    case GAIA_POINTM:
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // X
      p_in += sizeof( double );
      p_out += sizeof( double );
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // Y
      p_in += sizeof( double );
      p_out += sizeof( double );
      gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
      p_in += sizeof( double );
      p_out += sizeof( double );
      break;
    case GAIA_POINTZ:
    case GAIA_POINTZM:
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // X
      p_in += sizeof( double );
      p_out += sizeof( double );
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // Y
      p_in += sizeof( double );
      p_out += sizeof( double );
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // Z
      p_in += sizeof( double );
      p_out += sizeof( double );
      if ( type == GAIA_POINTZM )
        p_in += sizeof( double );
      break;
    case GAIA_LINESTRINGM:
      points = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, points, 1, endian_arch );
      p_out += 4;
      for ( iv = 0; iv < points; iv++ )
      {
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // X
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Y
        p_in += sizeof( double );
        p_out += sizeof( double );
        gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
        p_in += sizeof( double );
        p_out += sizeof( double );
      }
      break;
    case GAIA_LINESTRINGZ:
    case GAIA_LINESTRINGZM:
      points = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, points, 1, endian_arch );
      p_out += 4;
      for ( iv = 0; iv < points; iv++ )
      {
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // X
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Y
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Z
        p_in += sizeof( double );
        p_out += sizeof( double );
        if ( type == GAIA_LINESTRINGZM )
          p_in += sizeof( double );
      }
      break;
    case GAIA_POLYGONM:
      rings = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, rings, 1, endian_arch );
      p_out += 4;
      for ( ib = 0; ib < rings; ib++ )
      {
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, points, 1, endian_arch );
        p_out += 4;
        for ( iv = 0; iv < points; iv++ )
        {
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // X
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Y
          p_in += sizeof( double );
          p_out += sizeof( double );
          gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
          p_in += sizeof( double );
          p_out += sizeof( double );
        }
      }
      break;
    case GAIA_POLYGONZ:
    case GAIA_POLYGONZM:
      rings = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, rings, 1, endian_arch );
      p_out += 4;
      for ( ib = 0; ib < rings; ib++ )
      {
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, points, 1, endian_arch );
        p_out += 4;
        for ( iv = 0; iv < points; iv++ )
        {
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // X
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Y
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Z
          p_in += sizeof( double );
          p_out += sizeof( double );
          if ( type == GAIA_POLYGONZM )
            p_in += sizeof( double );
        }
      }
      break;
    case GAIA_MULTIPOINTM:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        gaiaExport32( p_out, GEOS_3D_POINT, 1, endian_arch );
        p_out += 4;
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // X
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Y
        p_in += sizeof( double );
        p_out += sizeof( double );
        gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
        p_in += sizeof( double );
        p_out += sizeof( double );
      }
      break;
    case GAIA_MULTIPOINTZ:
    case GAIA_MULTIPOINTZM:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        gaiaExport32( p_out, GEOS_3D_POINT, 1, endian_arch );
        p_out += 4;
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // X
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Y
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Z
        p_in += sizeof( double );
        p_out += sizeof( double );
        if ( type == GAIA_MULTIPOINTZM )
          p_in += sizeof( double );
      }
      break;
    case GAIA_MULTILINESTRINGM:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        gaiaExport32( p_out, GEOS_3D_LINESTRING, 1, endian_arch );
        p_out += 4;
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, points, 1, endian_arch );
        p_out += 4;
        for ( iv = 0; iv < points; iv++ )
        {
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // X
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Y
          p_in += sizeof( double );
          p_out += sizeof( double );
          gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
          p_in += sizeof( double );
          p_out += sizeof( double );
        }
      }
      break;
    case GAIA_MULTILINESTRINGZ:
    case GAIA_MULTILINESTRINGZM:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        gaiaExport32( p_out, GEOS_3D_LINESTRING, 1, endian_arch );
        p_out += 4;
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, points, 1, endian_arch );
        p_out += 4;
        for ( iv = 0; iv < points; iv++ )
        {
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // X
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Y
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Z
          p_in += sizeof( double );
          p_out += sizeof( double );
          if ( type == GAIA_MULTILINESTRINGZM )
            p_in += sizeof( double );
        }
      }
      break;
    case GAIA_MULTIPOLYGONM:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        gaiaExport32( p_out, GEOS_3D_POLYGON, 1, endian_arch );
        p_out += 4;
        rings = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, rings, 1, endian_arch );
        p_out += 4;
        for ( ib = 0; ib < rings; ib++ )
        {
          points = gaiaImport32( p_in, little_endian, endian_arch );
          p_in += 4;
          gaiaExport32( p_out, points, 1, endian_arch );
          p_out += 4;
          for ( iv = 0; iv < points; iv++ )
          {
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // X
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Y
            p_in += sizeof( double );
            p_out += sizeof( double );
            gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
            p_in += sizeof( double );
            p_out += sizeof( double );
          }
        }
      }
      break;
    case GAIA_MULTIPOLYGONZ:
    case GAIA_MULTIPOLYGONZM:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        gaiaExport32( p_out, GEOS_3D_POLYGON, 1, endian_arch );
        p_out += 4;
        rings = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, rings, 1, endian_arch );
        p_out += 4;
        for ( ib = 0; ib < rings; ib++ )
        {
          points = gaiaImport32( p_in, little_endian, endian_arch );
          p_in += 4;
          gaiaExport32( p_out, points, 1, endian_arch );
          p_out += 4;
          for ( iv = 0; iv < points; iv++ )
          {
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // X
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Y
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Z
            p_in += sizeof( double );
            p_out += sizeof( double );
            if ( type == GAIA_MULTIPOLYGONZM )
              p_in += sizeof( double );
          }
        }
      }
      break;
    case GAIA_GEOMETRYCOLLECTIONM:
    case GAIA_GEOMETRYCOLLECTIONZ:
    case GAIA_GEOMETRYCOLLECTIONZM:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        int type2 = gaiaImport32( p_in + 1, little_endian, endian_arch );
        p_in += 5;
        *p_out++ = 0x01;
        switch ( type2 )
        {
          case GAIA_POINTZ:
          case GAIA_POINTM:
          case GAIA_POINTZM:
            gaiaExport32( p_out, GEOS_3D_POINT, 1, endian_arch );
            break;
          case GAIA_LINESTRINGZ:
          case GAIA_LINESTRINGM:
          case GAIA_LINESTRINGZM:
            gaiaExport32( p_out, GEOS_3D_LINESTRING, 1, endian_arch );
            break;
          case GAIA_POLYGONZ:
          case GAIA_POLYGONM:
          case GAIA_POLYGONZM:
            gaiaExport32( p_out, GEOS_3D_POLYGON, 1, endian_arch );
            break;
        }
        p_out += 4;
        switch ( type2 )
        {
          // setting sub-Geometry values
          case GAIA_POINTM:
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // X
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Y
            p_in += sizeof( double );
            p_out += sizeof( double );
            gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
            p_in += sizeof( double );
            p_out += sizeof( double );
            break;
          case GAIA_POINTZ:
          case GAIA_POINTZM:
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // X
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Y
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Z
            p_in += sizeof( double );
            p_out += sizeof( double );
            if ( type2 == GAIA_POINTZM )
              p_in += sizeof( double );
            break;
          case GAIA_LINESTRINGM:
            points = gaiaImport32( p_in, little_endian, endian_arch );
            p_in += 4;
            gaiaExport32( p_out, points, 1, endian_arch );
            p_out += 4;
            for ( iv = 0; iv < points; iv++ )
            {
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // X
              p_in += sizeof( double );
              p_out += sizeof( double );
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // Y
              p_in += sizeof( double );
              p_out += sizeof( double );
              gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
              p_in += sizeof( double );
              p_out += sizeof( double );
            }
            break;
          case GAIA_LINESTRINGZ:
          case GAIA_LINESTRINGZM:
            points = gaiaImport32( p_in, little_endian, endian_arch );
            p_in += 4;
            gaiaExport32( p_out, points, 1, endian_arch );
            p_out += 4;
            for ( iv = 0; iv < points; iv++ )
            {
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // X
              p_in += sizeof( double );
              p_out += sizeof( double );
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // Y
              p_in += sizeof( double );
              p_out += sizeof( double );
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // Z
              p_in += sizeof( double );
              p_out += sizeof( double );
              if ( type2 == GAIA_LINESTRINGZM )
                p_in += sizeof( double );
            }
            break;
          case GAIA_POLYGONM:
            rings = gaiaImport32( p_in, little_endian, endian_arch );
            p_in += 4;
            gaiaExport32( p_out, rings, 1, endian_arch );
            p_out += 4;
            for ( ib = 0; ib < rings; ib++ )
            {
              points = gaiaImport32( p_in, little_endian, endian_arch );
              p_in += 4;
              gaiaExport32( p_out, points, 1, endian_arch );
              p_out += 4;
              for ( iv = 0; iv < points; iv++ )
              {
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // X
                p_in += sizeof( double );
                p_out += sizeof( double );
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // Y
                p_in += sizeof( double );
                p_out += sizeof( double );
                gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
                p_in += sizeof( double );
                p_out += sizeof( double );
              }
            }
            break;
          case GAIA_POLYGONZ:
          case GAIA_POLYGONZM:
            rings = gaiaImport32( p_in, little_endian, endian_arch );
            p_in += 4;
            gaiaExport32( p_out, rings, 1, endian_arch );
            p_out += 4;
            for ( ib = 0; ib < rings; ib++ )
            {
              points = gaiaImport32( p_in, little_endian, endian_arch );
              p_in += 4;
              gaiaExport32( p_out, points, 1, endian_arch );
              p_out += 4;
              for ( iv = 0; iv < points; iv++ )
              {
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // X
                p_in += sizeof( double );
                p_out += sizeof( double );
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // Y
                p_in += sizeof( double );
                p_out += sizeof( double );
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // Z
                p_in += sizeof( double );
                p_out += sizeof( double );
                if ( type2 == GAIA_POLYGONZM )
                  p_in += sizeof( double );
              }
            }
            break;
        }
      }
      break;
  }
  *wkb = wkbGeom;
  *geom_size = gsize;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteUtils::computeMultiWKB3Dsize
//-----------------------------------------------------------------
int QgsSpatiaLiteUtils::computeMultiWKB3Dsize( const unsigned char *p_in, int little_endian, int endian_arch )
{
// computing the required size to store a GEOS 3D MultiXX
  int entities;
  int type;
  int rings;
  int points;
  int ie;
  int ib;
  int size = 0;

  entities = gaiaImport32( p_in, little_endian, endian_arch );
  p_in += 4;
  size += 4;
  for ( ie = 0; ie < entities; ie++ )
  {
    type = gaiaImport32( p_in + 1, little_endian, endian_arch );
    p_in += 5;
    size += 5;
    switch ( type )
    {
      // compunting the required size
      case GAIA_POINTZ:
      case GAIA_POINTM:
        size += 3 * sizeof( double );
        p_in += 3 * sizeof( double );
        break;
      case GAIA_POINTZM:
        size += 3 * sizeof( double );
        p_in += 4 * sizeof( double );
        break;
      case GAIA_LINESTRINGZ:
      case GAIA_LINESTRINGM:
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        size += 4;
        size += points * ( 3 * sizeof( double ) );
        p_in += points * ( 3 * sizeof( double ) );
        break;
      case GAIA_LINESTRINGZM:
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        size += 4;
        size += points * ( 3 * sizeof( double ) );
        p_in += points * ( 4 * sizeof( double ) );
        break;
      case GAIA_POLYGONZ:
      case GAIA_POLYGONM:
        rings = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        size += 4;
        for ( ib = 0; ib < rings; ib++ )
        {
          points = gaiaImport32( p_in, little_endian, endian_arch );
          p_in += 4;
          size += 4;
          size += points * ( 3 * sizeof( double ) );
          p_in += points * ( 3 * sizeof( double ) );
        }
        break;
      case GAIA_POLYGONZM:
        rings = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        size += 4;
        for ( ib = 0; ib < rings; ib++ )
        {
          points = gaiaImport32( p_in, little_endian, endian_arch );
          p_in += 4;
          size += 4;
          size += points * ( 3 * sizeof( double ) );
          p_in += points * ( 4 * sizeof( double ) );
        }
        break;
    }
  }
  return size;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteUtils::computeSizeFromMultiWKB2D
//-----------------------------------------------------------------
int QgsSpatiaLiteUtils::computeSizeFromMultiWKB2D( const unsigned char *p_in,
    int nDims,
    int little_endian,
    int endian_arch )
{
// calculating the size required to store this WKB
  int entities;
  int type;
  int rings;
  int points;
  int ie;
  int ib;
  int size = 0;

  entities = gaiaImport32( p_in, little_endian, endian_arch );
  p_in += 4;
  size += 4;
  for ( ie = 0; ie < entities; ie++ )
  {
    type = gaiaImport32( p_in + 1, little_endian, endian_arch );
    p_in += 5;
    size += 5;
    switch ( type )
    {
      // compunting the required size
      case GAIA_POINT:
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            size += 4 * sizeof( double );
            break;
          case GAIA_XY_Z:
          case GAIA_XY_M:
            size += 3 * sizeof( double );
            break;
          default:
            size += 2 * sizeof( double );
            break;
        }
        p_in += 2 * sizeof( double );
        break;
      case GAIA_LINESTRING:
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        size += 4;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            size += points * ( 4 * sizeof( double ) );
            break;
          case GAIA_XY_Z:
          case GAIA_XY_M:
            size += points * ( 3 * sizeof( double ) );
            break;
          default:
            size += points * ( 2 * sizeof( double ) );
            break;
        }
        p_in += points * ( 2 * sizeof( double ) );
        break;
      case GAIA_POLYGON:
        rings = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        size += 4;
        for ( ib = 0; ib < rings; ib++ )
        {
          points = gaiaImport32( p_in, little_endian, endian_arch );
          p_in += 4;
          size += 4;
          switch ( nDims )
          {
            case GAIA_XY_Z_M:
              size += points * ( 4 * sizeof( double ) );
              break;
            case GAIA_XY_Z:
            case GAIA_XY_M:
              size += points * ( 3 * sizeof( double ) );
              break;
            default:
              size += points * ( 2 * sizeof( double ) );
              break;
          }
          p_in += points * ( 2 * sizeof( double ) );
        }
        break;
    }
  }
  return size;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteUtils::computeSizeFromMultiWKB3D
//-----------------------------------------------------------------
int QgsSpatiaLiteUtils::computeSizeFromMultiWKB3D( const unsigned char *p_in,
    int nDims,
    int little_endian,
    int endian_arch )
{
// calculating the size required to store this WKB
  int entities;
  int type;
  int rings;
  int points;
  int ie;
  int ib;
  int size = 0;

  entities = gaiaImport32( p_in, little_endian, endian_arch );
  p_in += 4;
  size += 4;
  for ( ie = 0; ie < entities; ie++ )
  {
    type = gaiaImport32( p_in + 1, little_endian, endian_arch );
    p_in += 5;
    size += 5;
    switch ( type )
    {
      // compunting the required size
      case GEOS_3D_POINT:
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            size += 4 * sizeof( double );
            break;
          case GAIA_XY_Z:
          case GAIA_XY_M:
            size += 3 * sizeof( double );
            break;
          default:
            size += 2 * sizeof( double );
            break;
        }
        p_in += 3 * sizeof( double );
        break;
      case GEOS_3D_LINESTRING:
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        size += 4;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            size += points * ( 4 * sizeof( double ) );
            break;
          case GAIA_XY_Z:
          case GAIA_XY_M:
            size += points * ( 3 * sizeof( double ) );
            break;
          default:
            size += points * ( 2 * sizeof( double ) );
            break;
        }
        p_in += points * ( 3 * sizeof( double ) );
        break;
      case GEOS_3D_POLYGON:
        rings = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        size += 4;
        for ( ib = 0; ib < rings; ib++ )
        {
          points = gaiaImport32( p_in, little_endian, endian_arch );
          p_in += 4;
          size += 4;
          switch ( nDims )
          {
            case GAIA_XY_Z_M:
              size += points * ( 4 * sizeof( double ) );
              break;
            case GAIA_XY_Z:
            case GAIA_XY_M:
              size += points * ( 3 * sizeof( double ) );
              break;
            default:
              size += points * ( 2 * sizeof( double ) );
              break;
          }
          p_in += points * ( 3 * sizeof( double ) );
        }
        break;
    }
  }
  return size;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteUtils::convertFromGeosWKB2D
//-----------------------------------------------------------------
void QgsSpatiaLiteUtils::convertFromGeosWKB2D( const unsigned char *blob,
    int blob_size,
    unsigned char *wkb,
    int geom_size,
    int nDims,
    int little_endian,
    int endian_arch )
{
  Q_UNUSED( blob_size );
  Q_UNUSED( geom_size );
// attempting to convert from 2D GEOS own WKB
  int type;
  int entities;
  int rings;
  int points;
  int ie;
  int ib;
  int iv;
  const unsigned char *p_in;
  unsigned char *p_out = wkb;
  double coord;

// building from GEOS 2D WKB
  *p_out++ = 0x01;  // little endian byte order
  type = gaiaImport32( blob + 1, little_endian, endian_arch );
  switch ( type )
  {
    // setting Geometry TYPE
    case GAIA_POINT:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_POINTZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_POINTZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_POINTM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_POINT, 1, endian_arch );
          break;
      }
      break;
    case GAIA_LINESTRING:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_LINESTRINGZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_LINESTRINGZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_LINESTRINGM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_LINESTRING, 1, endian_arch );
          break;
      }
      break;
    case GAIA_POLYGON:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_POLYGONZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_POLYGONZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_POLYGONM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_POLYGON, 1, endian_arch );
          break;
      }
      break;
    case GAIA_MULTIPOINT:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_MULTIPOINTZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_MULTIPOINTZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_MULTIPOINTM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_MULTIPOINT, 1, endian_arch );
          break;
      }
      break;
    case GAIA_MULTILINESTRING:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_MULTILINESTRINGZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_MULTILINESTRINGZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_MULTILINESTRINGM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_MULTILINESTRING, 1, endian_arch );
          break;
      }
      break;
    case GAIA_MULTIPOLYGON:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_MULTIPOLYGONZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_MULTIPOLYGONZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_MULTIPOLYGONM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_MULTIPOLYGON, 1, endian_arch );
          break;
      }
      break;
    case GAIA_GEOMETRYCOLLECTION:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_GEOMETRYCOLLECTIONZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_GEOMETRYCOLLECTIONZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_GEOMETRYCOLLECTIONM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_GEOMETRYCOLLECTION, 1, endian_arch );
          break;
      }
      break;
  }
  p_in = blob + 5;
  p_out += 4;
  switch ( type )
  {
    // setting Geometry values
    case GAIA_POINT:
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // X
      p_in += sizeof( double );
      p_out += sizeof( double );
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // Y
      p_in += sizeof( double );
      p_out += sizeof( double );
      if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
      {
        gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
        p_out += sizeof( double );
      }
      if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
      {
        gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
        p_out += sizeof( double );
      }
      break;
    case GAIA_LINESTRING:
      points = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, points, 1, endian_arch );
      p_out += 4;
      for ( iv = 0; iv < points; iv++ )
      {
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // X
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Y
        p_in += sizeof( double );
        p_out += sizeof( double );
        if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
        {
          gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
          p_out += sizeof( double );
        }
        if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
        {
          gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
          p_out += sizeof( double );
        }
      }
      break;
    case GAIA_POLYGON:
      rings = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, rings, 1, endian_arch );
      p_out += 4;
      for ( ib = 0; ib < rings; ib++ )
      {
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, points, 1, endian_arch );
        p_out += 4;
        for ( iv = 0; iv < points; iv++ )
        {
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // X
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Y
          p_in += sizeof( double );
          p_out += sizeof( double );
          if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
          {
            gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
            p_out += sizeof( double );
          }
          if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
          {
            gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
            p_out += sizeof( double );
          }
        }
      }
      break;
    case GAIA_MULTIPOINT:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            gaiaExport32( p_out, GAIA_POINTZM, 1, endian_arch );
            break;
          case GAIA_XY_Z:
            gaiaExport32( p_out, GAIA_POINTZ, 1, endian_arch );
            break;
          case GAIA_XY_M:
            gaiaExport32( p_out, GAIA_POINTM, 1, endian_arch );
            break;
          default:
            gaiaExport32( p_out, GAIA_POINT, 1, endian_arch );
            break;
        }
        p_out += 4;
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // X
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Y
        p_in += sizeof( double );
        p_out += sizeof( double );
        if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
        {
          gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
          p_out += sizeof( double );
        }
        if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
        {
          gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
          p_out += sizeof( double );
        }
      }
      break;
    case GAIA_MULTILINESTRING:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            gaiaExport32( p_out, GAIA_LINESTRINGZM, 1, endian_arch );
            break;
          case GAIA_XY_Z:
            gaiaExport32( p_out, GAIA_LINESTRINGZ, 1, endian_arch );
            break;
          case GAIA_XY_M:
            gaiaExport32( p_out, GAIA_LINESTRINGM, 1, endian_arch );
            break;
          default:
            gaiaExport32( p_out, GAIA_LINESTRING, 1, endian_arch );
            break;
        }
        p_out += 4;
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, points, 1, endian_arch );
        p_out += 4;
        for ( iv = 0; iv < points; iv++ )
        {
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // X
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Y
          p_in += sizeof( double );
          p_out += sizeof( double );
          if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
          {
            gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
            p_out += sizeof( double );
          }
          if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
          {
            gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
            p_out += sizeof( double );
          }
        }
      }
      break;
    case GAIA_MULTIPOLYGON:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            gaiaExport32( p_out, GAIA_POLYGONZM, 1, endian_arch );
            break;
          case GAIA_XY_Z:
            gaiaExport32( p_out, GAIA_POLYGONZ, 1, endian_arch );
            break;
          case GAIA_XY_M:
            gaiaExport32( p_out, GAIA_POLYGONM, 1, endian_arch );
            break;
          default:
            gaiaExport32( p_out, GAIA_POLYGON, 1, endian_arch );
            break;
        }
        p_out += 4;
        rings = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, rings, 1, endian_arch );
        p_out += 4;
        for ( ib = 0; ib < rings; ib++ )
        {
          points = gaiaImport32( p_in, little_endian, endian_arch );
          p_in += 4;
          gaiaExport32( p_out, points, 1, endian_arch );
          p_out += 4;
          for ( iv = 0; iv < points; iv++ )
          {
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // X
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Y
            p_in += sizeof( double );
            p_out += sizeof( double );
            if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
            {
              gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
              p_out += sizeof( double );
            }
            if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
            {
              gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
              p_out += sizeof( double );
            }
          }
        }
      }
      break;
    case GAIA_GEOMETRYCOLLECTION:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        int type2 = gaiaImport32( p_in + 1, little_endian, endian_arch );
        p_in += 5;
        *p_out++ = 0x01;
        switch ( type2 )
        {
          case GAIA_POINT:
            switch ( nDims )
            {
              case GAIA_XY_Z_M:
                gaiaExport32( p_out, GAIA_POINTZM, 1, endian_arch );
                break;
              case GAIA_XY_Z:
                gaiaExport32( p_out, GAIA_POINTZ, 1, endian_arch );
                break;
              case GAIA_XY_M:
                gaiaExport32( p_out, GAIA_POINTM, 1, endian_arch );
                break;
              default:
                gaiaExport32( p_out, GAIA_POINT, 1, endian_arch );
                break;
            }
            break;
          case GAIA_LINESTRING:
            switch ( nDims )
            {
              case GAIA_XY_Z_M:
                gaiaExport32( p_out, GAIA_LINESTRINGZM, 1, endian_arch );
                break;
              case GAIA_XY_Z:
                gaiaExport32( p_out, GAIA_LINESTRINGZ, 1, endian_arch );
                break;
              case GAIA_XY_M:
                gaiaExport32( p_out, GAIA_LINESTRINGM, 1, endian_arch );
                break;
              default:
                gaiaExport32( p_out, GAIA_LINESTRING, 1, endian_arch );
                break;
            }
            break;
          case GAIA_POLYGON:
            switch ( nDims )
            {
              case GAIA_XY_Z_M:
                gaiaExport32( p_out, GAIA_POLYGONZM, 1, endian_arch );
                break;
              case GAIA_XY_Z:
                gaiaExport32( p_out, GAIA_POLYGONZ, 1, endian_arch );
                break;
              case GAIA_XY_M:
                gaiaExport32( p_out, GAIA_POLYGONM, 1, endian_arch );
                break;
              default:
                gaiaExport32( p_out, GAIA_POLYGON, 1, endian_arch );
                break;
            }
            break;
        }
        p_out += 4;
        switch ( type2 )
        {
          // setting sub-Geometry values
          case GAIA_POINT:
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // X
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Y
            p_in += sizeof( double );
            p_out += sizeof( double );
            if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
            {
              gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
              p_out += sizeof( double );
            }
            if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
            {
              gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
              p_out += sizeof( double );
            }
            break;
          case GAIA_LINESTRING:
            points = gaiaImport32( p_in, little_endian, endian_arch );
            p_in += 4;
            gaiaExport32( p_out, points, 1, endian_arch );
            p_out += 4;
            for ( iv = 0; iv < points; iv++ )
            {
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // X
              p_in += sizeof( double );
              p_out += sizeof( double );
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // Y
              p_in += sizeof( double );
              p_out += sizeof( double );
              if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
              {
                gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
                p_out += sizeof( double );
              }
              if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
              {
                gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
                p_out += sizeof( double );
              }
            }
            break;
          case GAIA_POLYGON:
            rings = gaiaImport32( p_in, little_endian, endian_arch );
            p_in += 4;
            gaiaExport32( p_out, rings, 1, endian_arch );
            p_out += 4;
            for ( ib = 0; ib < rings; ib++ )
            {
              points = gaiaImport32( p_in, little_endian, endian_arch );
              p_in += 4;
              gaiaExport32( p_out, points, 1, endian_arch );
              p_out += 4;
              for ( iv = 0; iv < points; iv++ )
              {
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // X
                p_in += sizeof( double );
                p_out += sizeof( double );
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // Y
                p_in += sizeof( double );
                p_out += sizeof( double );
                if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
                {
                  gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
                  p_out += sizeof( double );
                }
                if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
                {
                  gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
                  p_out += sizeof( double );
                }
              }
            }
            break;
        }
      }
      break;
  }
}
//-----------------------------------------------------------------
// QgsSpatiaLiteUtils::convertFromGeosWKB3D
//-----------------------------------------------------------------
void QgsSpatiaLiteUtils::convertFromGeosWKB3D( const unsigned char *blob,
    int blob_size,
    unsigned char *wkb,
    int geom_size,
    int nDims,
    int little_endian,
    int endian_arch )
{
  Q_UNUSED( blob_size );
  Q_UNUSED( geom_size );
// attempting to convert from 3D GEOS own WKB
  int type;
  int entities;
  int rings;
  int points;
  int ie;
  int ib;
  int iv;
  const unsigned char *p_in;
  unsigned char *p_out = wkb;
  double coord;

// building from GEOS 3D WKB
  *p_out++ = 0x01;  // little endian byte order
  type = gaiaImport32( blob + 1, little_endian, endian_arch );
  switch ( type )
  {
    // setting Geometry TYPE
    case GEOS_3D_POINT:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_POINTZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_POINTZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_POINTM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_POINT, 1, endian_arch );
          break;
      }
      break;
    case GEOS_3D_LINESTRING:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_LINESTRINGZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_LINESTRINGZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_LINESTRINGM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_LINESTRING, 1, endian_arch );
          break;
      }
      break;
    case GEOS_3D_POLYGON:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_POLYGONZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_POLYGONZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_POLYGONM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_POLYGON, 1, endian_arch );
          break;
      }
      break;
    case GEOS_3D_MULTIPOINT:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_MULTIPOINTZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_MULTIPOINTZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_MULTIPOINTM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_MULTIPOINT, 1, endian_arch );
          break;
      }
      break;
    case GEOS_3D_MULTILINESTRING:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_MULTILINESTRINGZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_MULTILINESTRINGZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_MULTILINESTRINGM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_MULTILINESTRING, 1, endian_arch );
          break;
      }
      break;
    case GEOS_3D_MULTIPOLYGON:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_MULTIPOLYGONZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_MULTIPOLYGONZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_MULTIPOLYGONM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_MULTIPOLYGON, 1, endian_arch );
          break;
      }
      break;
    case GEOS_3D_GEOMETRYCOLLECTION:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_GEOMETRYCOLLECTIONZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_GEOMETRYCOLLECTIONZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_GEOMETRYCOLLECTIONM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_GEOMETRYCOLLECTION, 1, endian_arch );
          break;
      }
      break;
  }
  p_in = blob + 5;
  p_out += 4;
  switch ( type )
  {
    // setting Geometry values
    case GEOS_3D_POINT:
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // X
      p_in += sizeof( double );
      p_out += sizeof( double );
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // Y
      p_in += sizeof( double );
      p_out += sizeof( double );
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      p_in += sizeof( double );
      if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
      {
        gaiaExport64( p_out, coord, 1, endian_arch );  // Z
        p_out += sizeof( double );
      }
      if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
      {
        gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
        p_out += sizeof( double );
      }
      break;
    case GEOS_3D_LINESTRING:
      points = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, points, 1, endian_arch );
      p_out += 4;
      for ( iv = 0; iv < points; iv++ )
      {
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // X
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Y
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        p_in += sizeof( double );
        if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
        {
          gaiaExport64( p_out, coord, 1, endian_arch );  // Z
          p_out += sizeof( double );
        }
        if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
        {
          gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
          p_out += sizeof( double );
        }
      }
      break;
    case GEOS_3D_POLYGON:
      rings = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, rings, 1, endian_arch );
      p_out += 4;
      for ( ib = 0; ib < rings; ib++ )
      {
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, points, 1, endian_arch );
        p_out += 4;
        for ( iv = 0; iv < points; iv++ )
        {
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // X
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Y
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          p_in += sizeof( double );
          if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
          {
            gaiaExport64( p_out, coord, 1, endian_arch );  // Z
            p_out += sizeof( double );
          }
          if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
          {
            gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
            p_out += sizeof( double );
          }
        }
      }
      break;
    case GEOS_3D_MULTIPOINT:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            gaiaExport32( p_out, GAIA_POINTZM, 1, endian_arch );
            break;
          case GAIA_XY_Z:
            gaiaExport32( p_out, GAIA_POINTZ, 1, endian_arch );
            break;
          case GAIA_XY_M:
            gaiaExport32( p_out, GAIA_POINTM, 1, endian_arch );
            break;
          default:
            gaiaExport32( p_out, GAIA_POINT, 1, endian_arch );
            break;
        }
        p_out += 4;
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // X
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Y
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        p_in += sizeof( double );
        if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
        {
          gaiaExport64( p_out, coord, 1, endian_arch );  // Z
          p_out += sizeof( double );
        }
        if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
        {
          gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
          p_out += sizeof( double );
        }
      }
      break;
    case GEOS_3D_MULTILINESTRING:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            gaiaExport32( p_out, GAIA_LINESTRINGZM, 1, endian_arch );
            break;
          case GAIA_XY_Z:
            gaiaExport32( p_out, GAIA_LINESTRINGZ, 1, endian_arch );
            break;
          case GAIA_XY_M:
            gaiaExport32( p_out, GAIA_LINESTRINGM, 1, endian_arch );
            break;
          default:
            gaiaExport32( p_out, GAIA_LINESTRING, 1, endian_arch );
            break;
        }
        p_out += 4;
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, points, 1, endian_arch );
        p_out += 4;
        for ( iv = 0; iv < points; iv++ )
        {
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // X
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Y
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          p_in += sizeof( double );
          if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
          {
            gaiaExport64( p_out, coord, 1, endian_arch );  // Z
            p_out += sizeof( double );
          }
          if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
          {
            gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
            p_out += sizeof( double );
          }
        }
      }
      break;
    case GEOS_3D_MULTIPOLYGON:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            gaiaExport32( p_out, GAIA_POLYGONZM, 1, endian_arch );
            break;
          case GAIA_XY_Z:
            gaiaExport32( p_out, GAIA_POLYGONZ, 1, endian_arch );
            break;
          case GAIA_XY_M:
            gaiaExport32( p_out, GAIA_POLYGONM, 1, endian_arch );
            break;
          default:
            gaiaExport32( p_out, GAIA_POLYGON, 1, endian_arch );
            break;
        }
        p_out += 4;
        rings = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, rings, 1, endian_arch );
        p_out += 4;
        for ( ib = 0; ib < rings; ib++ )
        {
          points = gaiaImport32( p_in, little_endian, endian_arch );
          p_in += 4;
          gaiaExport32( p_out, points, 1, endian_arch );
          p_out += 4;
          for ( iv = 0; iv < points; iv++ )
          {
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // X
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Y
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            p_in += sizeof( double );
            if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
            {
              gaiaExport64( p_out, coord, 1, endian_arch );  // Z
              p_out += sizeof( double );
            }
            if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
            {
              gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
              p_out += sizeof( double );
            }
          }
        }
      }
      break;
    case GEOS_3D_GEOMETRYCOLLECTION:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        int type2 = gaiaImport32( p_in + 1, little_endian, endian_arch );
        p_in += 5;
        *p_out++ = 0x01;
        switch ( type2 )
        {
          case GEOS_3D_POINT:
            switch ( nDims )
            {
              case GAIA_XY_Z_M:
                gaiaExport32( p_out, GAIA_POINTZM, 1, endian_arch );
                break;
              case GAIA_XY_Z:
                gaiaExport32( p_out, GAIA_POINTZ, 1, endian_arch );
                break;
              case GAIA_XY_M:
                gaiaExport32( p_out, GAIA_POINTM, 1, endian_arch );
                break;
              default:
                gaiaExport32( p_out, GAIA_POINT, 1, endian_arch );
                break;
            }
            break;
          case GEOS_3D_LINESTRING:
            switch ( nDims )
            {
              case GAIA_XY_Z_M:
                gaiaExport32( p_out, GAIA_LINESTRINGZM, 1, endian_arch );
                break;
              case GAIA_XY_Z:
                gaiaExport32( p_out, GAIA_LINESTRINGZ, 1, endian_arch );
                break;
              case GAIA_XY_M:
                gaiaExport32( p_out, GAIA_LINESTRINGM, 1, endian_arch );
                break;
              default:
                gaiaExport32( p_out, GAIA_LINESTRING, 1, endian_arch );
                break;
            }
            break;
          case GEOS_3D_POLYGON:
            switch ( nDims )
            {
              case GAIA_XY_Z_M:
                gaiaExport32( p_out, GAIA_POLYGONZM, 1, endian_arch );
                break;
              case GAIA_XY_Z:
                gaiaExport32( p_out, GAIA_POLYGONZ, 1, endian_arch );
                break;
              case GAIA_XY_M:
                gaiaExport32( p_out, GAIA_POLYGONM, 1, endian_arch );
                break;
              default:
                gaiaExport32( p_out, GAIA_POLYGON, 1, endian_arch );
                break;
            }
            break;
        }
        p_out += 4;
        switch ( type2 )
        {
          // setting sub-Geometry values
          case GEOS_3D_POINT:
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // X
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Y
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            p_in += sizeof( double );
            if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
            {
              gaiaExport64( p_out, coord, 1, endian_arch );  // Z
              p_out += sizeof( double );
            }
            if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
            {
              gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
              p_out += sizeof( double );
            }
            break;
          case GEOS_3D_LINESTRING:
            points = gaiaImport32( p_in, little_endian, endian_arch );
            p_in += 4;
            gaiaExport32( p_out, points, 1, endian_arch );
            p_out += 4;
            for ( iv = 0; iv < points; iv++ )
            {
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // X
              p_in += sizeof( double );
              p_out += sizeof( double );
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // Y
              p_in += sizeof( double );
              p_out += sizeof( double );
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              p_in += sizeof( double );
              if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
              {
                gaiaExport64( p_out, coord, 1, endian_arch );  // Z
                p_out += sizeof( double );
              }
              if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
              {
                gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
                p_out += sizeof( double );
              }
            }
            break;
          case GEOS_3D_POLYGON:
            rings = gaiaImport32( p_in, little_endian, endian_arch );
            p_in += 4;
            gaiaExport32( p_out, rings, 1, endian_arch );
            p_out += 4;
            for ( ib = 0; ib < rings; ib++ )
            {
              points = gaiaImport32( p_in, little_endian, endian_arch );
              p_in += 4;
              gaiaExport32( p_out, points, 1, endian_arch );
              p_out += 4;
              for ( iv = 0; iv < points; iv++ )
              {
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // X
                p_in += sizeof( double );
                p_out += sizeof( double );
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // Y
                p_in += sizeof( double );
                p_out += sizeof( double );
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                p_in += sizeof( double );
                if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
                {
                  gaiaExport64( p_out, coord, 1, endian_arch );  // Z
                  p_out += sizeof( double );
                }
                if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
                {
                  gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
                  p_out += sizeof( double );
                }
              }
            }
            break;
        }
      }
      break;
  }
}
//-----------------------------------------------------------------
// QgsSpatiaLiteUtils::convertFromGeosWKB
//-----------------------------------------------------------------
void QgsSpatiaLiteUtils::convertFromGeosWKB( const unsigned char *blob,
    int blob_size,
    unsigned char **wkb,
    int *geom_size,
    int nDims )
{
// attempting to convert from 2D/3D GEOS own WKB
  int type;
  int gDims;
  int gsize;
  int little_endian;
  int endian_arch = gaiaEndianArch();

  *wkb = nullptr;
  *geom_size = 0;
  if ( blob_size < 5 )
    return;
  if ( *( blob + 0 ) == 0x01 )
    little_endian = GAIA_LITTLE_ENDIAN;
  else
    little_endian = GAIA_BIG_ENDIAN;
  type = gaiaImport32( blob + 1, little_endian, endian_arch );
  if ( type == GEOS_3D_POINT || type == GEOS_3D_LINESTRING
       || type == GEOS_3D_POLYGON
       || type == GEOS_3D_MULTIPOINT || type == GEOS_3D_MULTILINESTRING
       || type == GEOS_3D_MULTIPOLYGON || type == GEOS_3D_GEOMETRYCOLLECTION )
    gDims = 3;
  else if ( type == GAIA_POINT || type == GAIA_LINESTRING
            || type == GAIA_POLYGON || type == GAIA_MULTIPOINT
            || type == GAIA_MULTILINESTRING || type == GAIA_MULTIPOLYGON
            || type == GAIA_GEOMETRYCOLLECTION )
    gDims = 2;
  else
    return;

  if ( gDims == 2 && nDims == GAIA_XY )
  {
    // already 2D: simply copying is required
    unsigned char *wkbGeom = new unsigned char[blob_size + 1];
    memcpy( wkbGeom, blob, blob_size );
    memset( wkbGeom + blob_size, 0, 1 );
    *wkb = wkbGeom;
    *geom_size = blob_size + 1;
    return;
  }

// we need creating a GAIA WKB
  if ( gDims == 3 )
    gsize = QgsSpatiaLiteUtils::computeSizeFromGeosWKB3D( blob, blob_size, type, nDims,
            little_endian, endian_arch );
  else
    gsize = QgsSpatiaLiteUtils::computeSizeFromGeosWKB2D( blob, blob_size, type, nDims,
            little_endian, endian_arch );

  unsigned char *wkbGeom = new unsigned char[gsize];
  memset( wkbGeom, '\0', gsize );

  if ( gDims == 3 )
    QgsSpatiaLiteUtils::convertFromGeosWKB3D( blob, blob_size, wkbGeom, gsize, nDims,
        little_endian, endian_arch );
  else
    QgsSpatiaLiteUtils::convertFromGeosWKB2D( blob, blob_size, wkbGeom, gsize, nDims,
        little_endian, endian_arch );

  *wkb = wkbGeom;
  *geom_size = gsize;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteUtils::computeSizeFromGeosWKB3D
//-----------------------------------------------------------------
int QgsSpatiaLiteUtils::computeSizeFromGeosWKB3D( const unsigned char *blob,
    int size, int type, int nDims,
    int little_endian, int endian_arch )
{
  Q_UNUSED( size );
// calculating the size required to store this WKB
  int rings;
  int points;
  int ib;
  const unsigned char *p_in = blob + 5;
  int gsize = 5;

  switch ( type )
  {
    // compunting the required size
    case GEOS_3D_POINT:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gsize += 4 * sizeof( double );
          break;
        case GAIA_XY_M:
        case GAIA_XY_Z:
          gsize += 3 * sizeof( double );
          break;
        default:
          gsize += 2 * sizeof( double );
          break;
      }
      break;
    case GEOS_3D_LINESTRING:
      points = gaiaImport32( p_in, little_endian, endian_arch );
      gsize += 4;
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gsize += points * ( 4 * sizeof( double ) );
          break;
        case GAIA_XY_M:
        case GAIA_XY_Z:
          gsize += points * ( 3 * sizeof( double ) );
          break;
        default:
          gsize += points * ( 2 * sizeof( double ) );
          break;
      }
      break;
    case GEOS_3D_POLYGON:
      rings = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gsize += 4;
      for ( ib = 0; ib < rings; ib++ )
      {
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gsize += 4;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            gsize += points * ( 4 * sizeof( double ) );
            break;
          case GAIA_XY_M:
          case GAIA_XY_Z:
            gsize += points * ( 3 * sizeof( double ) );
            break;
          default:
            gsize += points * ( 2 * sizeof( double ) );
            break;
        }
        p_in += points * ( 3 * sizeof( double ) );
      }
      break;
    default:
      gsize += QgsSpatiaLiteUtils::computeSizeFromMultiWKB3D( p_in, nDims, little_endian,
               endian_arch );
      break;
  }
  return gsize;
}
//-----------------------------------------------------------
int QgsSpatiaLiteUtils::computeSizeFromGeosWKB2D( const unsigned char *blob,
    int size, int type, int nDims, int little_endian, int endian_arch )
{
  Q_UNUSED( size );
// calculating the size required to store this WKB
  int rings;
  int points;
  int ib;
  const unsigned char *p_in = blob + 5;
  int gsize = 5;

  switch ( type )
  {
    // compunting the required size
    case GAIA_POINT:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gsize += 4 * sizeof( double );
          break;
        case GAIA_XY_M:
        case GAIA_XY_Z:
          gsize += 3 * sizeof( double );
          break;
        default:
          gsize += 2 * sizeof( double );
          break;
      }
      break;
    case GAIA_LINESTRING:
      points = gaiaImport32( p_in, little_endian, endian_arch );
      gsize += 4;
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gsize += points * ( 4 * sizeof( double ) );
          break;
        case GAIA_XY_M:
        case GAIA_XY_Z:
          gsize += points * ( 3 * sizeof( double ) );
          break;
        default:
          gsize += points * ( 2 * sizeof( double ) );
          break;
      }
      break;
    case GAIA_POLYGON:
      rings = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gsize += 4;
      for ( ib = 0; ib < rings; ib++ )
      {
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gsize += 4;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            gsize += points * ( 4 * sizeof( double ) );
            break;
          case GAIA_XY_M:
          case GAIA_XY_Z:
            gsize += points * ( 3 * sizeof( double ) );
            break;
          default:
            gsize += points * ( 2 * sizeof( double ) );
            break;
        }
        p_in += points * ( 2 * sizeof( double ) );
      }
      break;
    default:
      gsize += QgsSpatiaLiteUtils::computeSizeFromMultiWKB2D( p_in, nDims, little_endian, endian_arch );
      break;
  }
  return gsize;
}
//-----------------------------------------------------------
