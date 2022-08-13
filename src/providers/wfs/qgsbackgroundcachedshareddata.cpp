/***************************************************************************
    qgsbackgroundcachedshareddata.cpp
    ---------------------
    begin                : October 2019
    copyright            : (C) 2016-2019 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsbackgroundcachedshareddata.h"
#include "qgsbackgroundcachedfeatureiterator.h"

#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsproviderregistry.h"
#include "qgsspatialiteutils.h"
#include "qgsvectorfilewriter.h"
#include "qgswfsutils.h" // for isCompatibleType()

#include <QCryptographicHash>
#include <QDir>
#include <QMutex>

#include <set>

#include <cpl_vsi.h>
#include <cpl_conv.h>
#include <gdal.h>
#include <ogr_api.h>

#include <sqlite3.h>

QgsBackgroundCachedSharedData::QgsBackgroundCachedSharedData(
  const QString &providerName, const QString &componentTranslated ):
  mCacheDirectoryManager( QgsCacheDirectoryManager::singleton( ( providerName ) ) ),
  mComponentTranslated( componentTranslated )
{
}

QgsBackgroundCachedSharedData::~QgsBackgroundCachedSharedData()
{
  QgsDebugMsgLevel( QStringLiteral( "~QgsBackgroundCachedSharedData()" ), 4 );

  // Check that cleanup() has been called by implementations !
  Q_ASSERT( mCacheIdDb == nullptr );
  Q_ASSERT( mCacheIdDbname.isEmpty() );
}

void QgsBackgroundCachedSharedData::copyStateToClone( QgsBackgroundCachedSharedData *clone ) const
{
  clone->mFields = mFields;
  clone->mSourceCrs = mSourceCrs;
  clone->mDistinctSelect = mDistinctSelect;
  clone->mClientSideFilterExpression = mClientSideFilterExpression;
  clone->mMaxFeatures = mMaxFeatures;
  clone->mServerMaxFeatures = mServerMaxFeatures;
  clone->mCapabilityExtent = mCapabilityExtent;
  clone->mComputedExtent = mComputedExtent;
  clone->mHasNumberMatched = mHasNumberMatched;
  clone->mHideProgressDialog = mHideProgressDialog;
}

void QgsBackgroundCachedSharedData::cleanup()
{
  invalidateCache();

  mCacheIdDb.reset();
  if ( !mCacheIdDbname.isEmpty() )
  {
    QFile::remove( mCacheIdDbname );
    QFile::remove( mCacheIdDbname + "-wal" );
    QFile::remove( mCacheIdDbname + "-shm" );
    releaseCacheDirectory();
    mCacheIdDbname.clear();
  }
}

QString QgsBackgroundCachedSharedData::acquireCacheDirectory()
{
  return mCacheDirectoryManager.acquireCacheDirectory();
}

void QgsBackgroundCachedSharedData::releaseCacheDirectory()
{
  mCacheDirectoryManager.releaseCacheDirectory();
}

// This is called by the destructor or provider's reloadData(). The effect is to invalid
// all the caching state, so that a new request results in fresh download
void QgsBackgroundCachedSharedData::invalidateCache()
{
  // Cf explanations in registerToCache() for the locking strategy
  QMutexLocker lockerMyself( &mMutexRegisterToCache );

  QMutexLocker locker( &mMutex );

// to prevent deadlock when waiting the end of the downloader thread that will try to take the mutex in serializeFeatures()
  mMutex.unlock();
  mDownloader.reset();
  mMutex.lock();
  mDownloadFinished = false;
  mGenCounter = 0;
  mCachedRegions = QgsSpatialIndex();
  mRegions.clear();
  mRect = QgsRectangle();
  mComputedExtent = QgsRectangle();
  mRequestLimit = 0;
  mFeatureCount = 0;
  mFeatureCountExact = false;
  mFeatureCountRequestIssued = false;
  mTotalFeaturesAttemptedToBeCached = 0;
  if ( !mCacheDbname.isEmpty() && mCacheDataProvider )
  {
    // We need to invalidate connections pointing to the cache, so as to
    // be able to delete the file.
    mCacheDataProvider->invalidateConnections( mCacheDbname );
  }
  mCacheDataProvider.reset();

  if ( !mCacheDbname.isEmpty() )
  {
    QFile::remove( mCacheDbname );
    QFile::remove( mCacheDbname + "-wal" );
    QFile::remove( mCacheDbname + "-shm" );
    mCacheDbname.clear();
  }

  invalidateCacheBaseUnderLock();
}

QString QgsBackgroundCachedSharedData::getSpatialiteFieldNameFromUserVisibleName( const QString &columnName ) const
{
  auto oIter = mMapUserVisibleFieldNameToSpatialiteColumnName.find( columnName );
  Q_ASSERT( oIter != mMapUserVisibleFieldNameToSpatialiteColumnName.end() );
  return oIter->second;
}

bool QgsBackgroundCachedSharedData::getUserVisibleIdFromSpatialiteId( QgsFeatureId dbId, QgsFeatureId &outId ) const
{
  // Retrieve the user-visible id from the Spatialite cache database Id
  if ( mCacheIdDb.get() )
  {
    QString sql = qgs_sqlite3_mprintf( "SELECT qgisId FROM id_cache WHERE dbId = %lld", dbId );
    int resultCode;
    auto stmt = mCacheIdDb.prepare( sql, resultCode );
    if ( stmt.step() == SQLITE_ROW )
    {
      outId = stmt.columnAsInt64( 0 );
      Q_ASSERT( stmt.step() != SQLITE_ROW );
      return true;
    }
  }
  return false;
}

static QString quotedIdentifier( QString id )
{
  id.replace( '\"', QLatin1String( "\"\"" ) );
  return id.prepend( '\"' ).append( '\"' );
}

bool QgsBackgroundCachedSharedData::createCache()
{
  Q_ASSERT( mCacheDbname.isEmpty() );

  static QAtomicInt sTmpCounter = 0;
  int tmpCounter = ++sTmpCounter;
  QString cacheDirectory( acquireCacheDirectory() );
  mCacheDbname = QDir( cacheDirectory ).filePath( QStringLiteral( "cache_%1.sqlite" ).arg( tmpCounter ) );
  Q_ASSERT( !QFile::exists( mCacheDbname ) );

  QgsFields cacheFields;
  std::set<QString> setSQLiteColumnNameUpperCase;
  for ( const QgsField &field : std::as_const( mFields ) )
  {
    QVariant::Type type = field.type();
    // Map DateTime to int64 milliseconds from epoch
    if ( type == QVariant::DateTime )
    {
      // Note: this is just a wish. If GDAL < 2, QgsVectorFileWriter will actually map
      // it to a String
      type = QVariant::LongLong;
    }
    else if ( type == QVariant::List && field.subType() == QVariant::String )
    {
      type = QVariant::StringList;
    }

    // Make sure we don't have several field names that only differ by their case
    QString sqliteFieldName( field.name() );
    int counter = 2;
    while ( setSQLiteColumnNameUpperCase.find( sqliteFieldName.toUpper() ) != setSQLiteColumnNameUpperCase.end() )
    {
      sqliteFieldName = field.name() + QString::number( counter );
      counter++;
    }
    setSQLiteColumnNameUpperCase.insert( sqliteFieldName.toUpper() );
    mMapUserVisibleFieldNameToSpatialiteColumnName[field.name()] = sqliteFieldName;

    cacheFields.append( QgsField( sqliteFieldName, type, field.typeName() ) );
  }
  // Add some field for our internal use
  cacheFields.append( QgsField( QgsBackgroundCachedFeatureIteratorConstants::FIELD_GEN_COUNTER, QVariant::Int, QStringLiteral( "int" ) ) );
  cacheFields.append( QgsField( QgsBackgroundCachedFeatureIteratorConstants::FIELD_UNIQUE_ID, QVariant::String, QStringLiteral( "string" ) ) );
  cacheFields.append( QgsField( QgsBackgroundCachedFeatureIteratorConstants::FIELD_HEXWKB_GEOM, QVariant::String, QStringLiteral( "string" ) ) );
  if ( mDistinctSelect )
    cacheFields.append( QgsField( QgsBackgroundCachedFeatureIteratorConstants::FIELD_MD5, QVariant::String, QStringLiteral( "string" ) ) );

  const auto logMessageWithReason = [this]( const QString & reason )
  {
    QgsMessageLog::logMessage( QStringLiteral( "%1: %2" ).arg( QObject::tr( "Cannot create temporary SpatiaLite cache." ) ).arg( reason ), mComponentTranslated );
  };

  // Creating a SpatiaLite database can be quite slow on some file systems
  // so we create a GDAL in-memory file, and then copy it on
  // the file system.
  GDALDriverH hDrv = GDALGetDriverByName( "SQLite" );
  if ( !hDrv )
  {
    logMessageWithReason( QStringLiteral( "GDAL SQLite driver not available" ) );
    return false;
  }
  const QString vsimemFilename = QStringLiteral( "/vsimem/qgis_cache_template_%1/features.sqlite" ).arg( reinterpret_cast< quintptr >( this ), QT_POINTER_SIZE * 2, 16, QLatin1Char( '0' ) );
  mCacheTablename = CPLGetBasename( vsimemFilename.toStdString().c_str() );
  VSIUnlink( vsimemFilename.toStdString().c_str() );
  const char *apszOptions[] = { "INIT_WITH_EPSG=NO", "SPATIALITE=YES", nullptr };
  GDALDatasetH hDS = GDALCreate( hDrv, vsimemFilename.toUtf8().constData(), 0, 0, 0, GDT_Unknown, const_cast<char **>( apszOptions ) );
  if ( !hDS )
  {
    logMessageWithReason( QStringLiteral( "GDALCreate() failed: %1" ).arg( CPLGetLastErrorMsg() ) );
    return false;
  }
  GDALClose( hDS );

  // Copy the temporary database back to disk
  vsi_l_offset nLength = 0;
  GByte *pabyData = VSIGetMemFileBuffer( vsimemFilename.toStdString().c_str(), &nLength, TRUE );
  Q_ASSERT( !QFile::exists( mCacheDbname ) );
  VSILFILE *fp = VSIFOpenL( mCacheDbname.toStdString().c_str(), "wb" );
  if ( fp )
  {
    VSIFWriteL( pabyData, 1, nLength, fp );
    VSIFCloseL( fp );
    CPLFree( pabyData );
  }
  else
  {
    CPLFree( pabyData );
    logMessageWithReason( QStringLiteral( "Cannot copy file to %1: %2" ).arg( mCacheDbname ).arg( CPLGetLastErrorMsg() ) );
    return false;
  }


  QString fidName( QStringLiteral( "__ogc_fid" ) );
  QString geometryFieldname( QStringLiteral( "__spatialite_geometry" ) );

  spatialite_database_unique_ptr database;
  bool ret = true;
  int rc = database.open( mCacheDbname );
  QString failedSql;
  if ( rc == SQLITE_OK )
  {
    QString sql;

    ( void )sqlite3_exec( database.get(), "PRAGMA synchronous=OFF", nullptr, nullptr, nullptr );
    // WAL is needed to avoid reader to block writers
    ( void )sqlite3_exec( database.get(), "PRAGMA journal_mode=WAL", nullptr, nullptr, nullptr );

    ( void )sqlite3_exec( database.get(), "BEGIN", nullptr, nullptr, nullptr );

    mCacheTablename = QStringLiteral( "features" );
    sql = QStringLiteral( "CREATE TABLE %1 (%2 INTEGER PRIMARY KEY" ).arg( mCacheTablename, fidName );

    for ( const QgsField &field : std::as_const( cacheFields ) )
    {
      QString type( QStringLiteral( "VARCHAR" ) );
      if ( field.type() == QVariant::Int )
        type = QStringLiteral( "INTEGER" );
      else if ( field.type() == QVariant::LongLong )
        type = QStringLiteral( "BIGINT" );
      else if ( field.type() == QVariant::Double )
        type = QStringLiteral( "REAL" );
      else if ( field.type() == QVariant::StringList )
        type = QStringLiteral( "JSONSTRINGLIST" );

      sql += QStringLiteral( ", %1 %2" ).arg( quotedIdentifier( field.name() ), type );
    }
    sql += QLatin1Char( ')' );
    rc = sqlite3_exec( database.get(), sql.toUtf8(), nullptr, nullptr, nullptr );
    if ( rc != SQLITE_OK )
    {
      QgsDebugMsg( QStringLiteral( "%1 failed" ).arg( sql ) );
      if ( failedSql.isEmpty() ) failedSql = sql;
      ret = false;
    }

    sql = QStringLiteral( "SELECT AddGeometryColumn('%1','%2',0,'POLYGON',2)" ).arg( mCacheTablename, geometryFieldname );
    rc = sqlite3_exec( database.get(), sql.toUtf8(), nullptr, nullptr, nullptr );
    if ( rc != SQLITE_OK )
    {
      QgsDebugMsg( QStringLiteral( "%1 failed" ).arg( sql ) );
      if ( failedSql.isEmpty() ) failedSql = sql;
      ret = false;
    }

    sql = QStringLiteral( "SELECT CreateSpatialIndex('%1','%2')" ).arg( mCacheTablename, geometryFieldname );
    rc = sqlite3_exec( database.get(), sql.toUtf8(), nullptr, nullptr, nullptr );
    if ( rc != SQLITE_OK )
    {
      QgsDebugMsg( QStringLiteral( "%1 failed" ).arg( sql ) );
      if ( failedSql.isEmpty() ) failedSql = sql;
      ret = false;
    }


    // We need an index on the uniqueId, since we will check for duplicates, particularly
    // useful in the case we do overlapping BBOX requests
    sql = QStringLiteral( "CREATE INDEX idx_%2 ON %1(%2)" ).arg( mCacheTablename, QgsBackgroundCachedFeatureIteratorConstants::FIELD_UNIQUE_ID );
    rc = sqlite3_exec( database.get(), sql.toUtf8(), nullptr, nullptr, nullptr );
    if ( rc != SQLITE_OK )
    {
      QgsDebugMsg( QStringLiteral( "%1 failed" ).arg( sql ) );
      if ( failedSql.isEmpty() ) failedSql = sql;
      ret = false;
    }

    if ( mDistinctSelect )
    {
      sql = QStringLiteral( "CREATE INDEX idx_%2 ON %1(%2)" ).arg( mCacheTablename, QgsBackgroundCachedFeatureIteratorConstants::FIELD_MD5 );
      rc = sqlite3_exec( database.get(), sql.toUtf8(), nullptr, nullptr, nullptr );
      if ( rc != SQLITE_OK )
      {
        QgsDebugMsg( QStringLiteral( "%1 failed" ).arg( sql ) );
        if ( failedSql.isEmpty() ) failedSql = sql;
        ret = false;
      }
    }

    ( void )sqlite3_exec( database.get(), "COMMIT", nullptr, nullptr, nullptr );
  }
  else
  {
    ret = false;
  }
  if ( !ret )
  {
    logMessageWithReason( QStringLiteral( "SQL request %1 failed" ).arg( failedSql ) );
    return false;
  }

  // Some pragmas to speed-up writing. We don't need much integrity guarantee
  // regarding crashes, since this is a temporary DB
  QgsDataSourceUri dsURI;
  dsURI.setDatabase( mCacheDbname );
  dsURI.setDataSource( QString(), mCacheTablename, geometryFieldname, QString(), fidName );
  QStringList pragmas;
  pragmas << QStringLiteral( "synchronous=OFF" );
  pragmas << QStringLiteral( "journal_mode=WAL" ); // WAL is needed to avoid reader to block writers
  dsURI.setParam( QStringLiteral( "pragma" ), pragmas );

  QgsDataProvider::ProviderOptions providerOptions;
  mCacheDataProvider.reset( dynamic_cast<QgsVectorDataProvider *>( QgsProviderRegistry::instance()->createProvider(
                              QStringLiteral( "spatialite" ), dsURI.uri(), providerOptions ) ) );
  if ( mCacheDataProvider && !mCacheDataProvider->isValid() )
  {
    mCacheDataProvider.reset();
  }
  if ( !mCacheDataProvider )
  {
    QgsMessageLog::logMessage( QObject::tr( "Cannot connect to temporary SpatiaLite cache" ), mComponentTranslated );
    return false;
  }

  // The id_cache should be generated once for the lifetime of QgsBackgroundCachedFeatureIteratorConstants
  // to ensure consistency of the ids returned to the user.
  if ( mCacheIdDbname.isEmpty() )
  {
    mCacheIdDbname = QDir( cacheDirectory ).filePath( QStringLiteral( "id_cache_%1.sqlite" ).arg( tmpCounter ) );
    Q_ASSERT( !QFile::exists( mCacheIdDbname ) );
    if ( mCacheIdDb.open( mCacheIdDbname ) != SQLITE_OK )
    {
      QgsMessageLog::logMessage( QObject::tr( "Cannot create temporary id cache" ), mComponentTranslated );
      return false;
    }
    QString errorMsg;
    bool ok = mCacheIdDb.exec( QStringLiteral( "PRAGMA synchronous=OFF" ), errorMsg ) == SQLITE_OK;
    // WAL is needed to avoid reader to block writers
    ok &= mCacheIdDb.exec( QStringLiteral( "PRAGMA journal_mode=WAL" ), errorMsg ) == SQLITE_OK;
    // uniqueId is the uniqueId or fid attribute coming from the GML GetFeature response
    // qgisId is the feature id of the features returned to QGIS. That one should remain the same for a given uniqueId even after a layer reload
    // dbId is the feature id of the Spatialite feature in mCacheDataProvider. It might change for a given uniqueId after a layer reload
    ok &= mCacheIdDb.exec( QStringLiteral( "CREATE TABLE id_cache(uniqueId TEXT, dbId INTEGER, qgisId INTEGER)" ), errorMsg ) == SQLITE_OK;
    ok &= mCacheIdDb.exec( QStringLiteral( "CREATE INDEX idx_uniqueId ON id_cache(uniqueId)" ), errorMsg ) == SQLITE_OK;
    ok &= mCacheIdDb.exec( QStringLiteral( "CREATE INDEX idx_dbId ON id_cache(dbId)" ), errorMsg ) == SQLITE_OK;
    ok &= mCacheIdDb.exec( QStringLiteral( "CREATE INDEX idx_qgisId ON id_cache(qgisId)" ), errorMsg ) == SQLITE_OK;
    if ( !ok )
    {
      QgsDebugMsg( errorMsg );
      return false;
    }
  }

  return true;
}

int QgsBackgroundCachedSharedData::registerToCache( QgsBackgroundCachedFeatureIterator *iterator, int limit, const QgsRectangle &rect )
{
  // This locks prevents 2 readers to register at the same time (and particularly
  // destroy the current mDownloader at the same time)
  // Why not using the mMutex lock ? Because we must unlock it temporarily later
  // in the function, when destroying the current downloader, so we must have
  // another lock to prevent another reader to go into the critical section.
  QMutexLocker lockerMyself( &mMutexRegisterToCache );

  // This lock prevents a reader and the downloader/writer to manipulate tmpCounter
  // together
  QMutexLocker locker( &mMutex );
  if ( mCacheDbname.isEmpty() )
  {
    if ( !createCache() )
    {
      return -1;
    }
  }

  // In case the request has a spatial filter, which is not the one currently
  // being downloaded, check if we have already downloaded an area of interest that includes it
  // before deciding to restart a new download with the provided area of interest.
  // But don't abort a on going download without a BBOX (Offline editing use case
  // when "Only request features overlapping the view extent" : the offline editor
  // want to request all features whereas the map renderer only the view)
  bool newDownloadNeeded = false;
  if ( !rect.isEmpty() && mRect != rect && !( mDownloader && mRect.isEmpty() ) )
  {
    QList<QgsFeatureId> intersectingRequests = mCachedRegions.intersects( rect );
    newDownloadNeeded = true;
    const auto constIntersectingRequests = intersectingRequests;
    for ( QgsFeatureId id : constIntersectingRequests )
    {
      Q_ASSERT( id >= 0 && id < mRegions.size() ); // by construction, but doesn't hurt to be checked

      // If the requested bbox is inside an already cached rect that didn't
      // hit the download limit, then we can reuse the cached features without
      // issuing a new request.
      if ( mRegions[id].geometry().boundingBox().contains( rect ) &&
           !mRegions[id].attributes().value( 0 ).toBool() )
      {
        QgsDebugMsgLevel( QStringLiteral( "Cached features already cover this area of interest" ), 4 );
        newDownloadNeeded = false;
        break;
      }

      // On the other hand, if the requested bbox is inside an already cached rect,
      // that hit the download limit, our larger bbox will hit it too, so no need
      // to re-issue a new request either.
      if ( rect.contains( mRegions[id].geometry().boundingBox() ) &&
           mRegions[id].attributes().value( 0 ).toBool() )
      {
        QgsDebugMsgLevel( QStringLiteral( "Current request is larger than a smaller request that hit the download limit, so no server download needed." ), 4 );
        newDownloadNeeded = false;
        break;
      }
    }
  }
  // If there's a ongoing download with a BBOX and we request a new download
  // without it, then we need a new download.
  else if ( rect.isEmpty() && mDownloader && !mRect.isEmpty() )
  {
    newDownloadNeeded = true;
  }
  // If there's a ongoing download with a limitation, and the new download is
  // unlimited, we need a new download.
  else if ( supportsLimitedFeatureCountDownloads() && limit <= 0 && mRequestLimit > 0 )
  {
    newDownloadNeeded = true;
  }

  if ( newDownloadNeeded || !mDownloader )
  {
    mRect = rect;
    mRequestLimit = ( limit > 0 && supportsLimitedFeatureCountDownloads() ) ? limit : 0;
    // to prevent deadlock when waiting the end of the downloader thread that will try to take the mutex in serializeFeatures()
    mMutex.unlock();
    mDownloader.reset();
    mMutex.lock();
    mDownloadFinished = false;
    mComputedExtent = QgsRectangle();
    mDownloader.reset( new QgsThreadedFeatureDownloader( this ) );
    mDownloader->startAndWait();
  }
  if ( mDownloadFinished )
    return -1;

  // Under the mutex to ensure that we will not miss features
  iterator->connectSignals( mDownloader->downloader() );

  return mGenCounter ++;
}

int QgsBackgroundCachedSharedData::getUpdatedCounter()
{
  QMutexLocker locker( &mMutex );
  if ( mDownloadFinished )
    return mGenCounter;
  return mGenCounter ++;
}

void QgsBackgroundCachedSharedData::serializeFeatures( QVector<QgsFeatureUniqueIdPair> &featureList )
{
  QgsDebugMsgLevel( QStringLiteral( "begin %1" ).arg( featureList.size() ), 4 );

  int genCounter;
  {
    QMutexLocker locker( &mMutex );
    if ( mCacheDbname.isEmpty() )
    {
      if ( !createCache() )
      {
        return;
      }
    }
    if ( !mCacheDataProvider )
    {
      return;
    }

    genCounter = mGenCounter;
  }

  QgsFeatureList featureListToCache;
  QgsFields dataProviderFields = mCacheDataProvider->fields();
  int uniqueIdIdx = dataProviderFields.indexFromName( QgsBackgroundCachedFeatureIteratorConstants::FIELD_UNIQUE_ID );
  Q_ASSERT( uniqueIdIdx >= 0 );
  int genCounterIdx = dataProviderFields.indexFromName( QgsBackgroundCachedFeatureIteratorConstants::FIELD_GEN_COUNTER );
  Q_ASSERT( genCounterIdx >= 0 );
  int hexwkbGeomIdx = dataProviderFields.indexFromName( QgsBackgroundCachedFeatureIteratorConstants::FIELD_HEXWKB_GEOM );
  Q_ASSERT( hexwkbGeomIdx >= 0 );
  int md5Idx = ( mDistinctSelect ) ? dataProviderFields.indexFromName( QgsBackgroundCachedFeatureIteratorConstants::FIELD_MD5 ) : -1;

  QSet<QString> existingUniqueIds;
  QSet<QString> existingMD5s;
  if ( mDistinctSelect )
    existingMD5s = getExistingCachedMD5( featureList );
  else
    existingUniqueIds = getExistingCachedUniqueIds( featureList );
  QVector<QgsFeatureUniqueIdPair> updatedFeatureList;

  QgsRectangle localComputedExtent( mComputedExtent );
  const auto constFeatureList = featureList;
  for ( const QgsFeatureUniqueIdPair &featPair : constFeatureList )
  {
    const QgsFeature &srcFeature = featPair.first;

    QgsFeature cachedFeature;
    cachedFeature.initAttributes( dataProviderFields.size() );

    // copy the geometry
    // Do this now to update localComputedExtent, even if we skip the feature
    // afterwards as being already downloaded.
    QgsGeometry geometry = srcFeature.geometry();
    if ( hasGeometry() && !geometry.isNull() )
    {
      QByteArray array( geometry.asWkb() );

      cachedFeature.setAttribute( hexwkbGeomIdx, QVariant( QString( array.toHex().data() ) ) );

      QgsRectangle bBox( geometry.boundingBox() );
      if ( localComputedExtent.isNull() )
        localComputedExtent = bBox;
      else
        localComputedExtent.combineExtentWith( bBox );
      QgsGeometry polyBoundingBox = QgsGeometry::fromRect( bBox );
      cachedFeature.setGeometry( polyBoundingBox );
    }
    else
    {
      cachedFeature.setAttribute( hexwkbGeomIdx, QVariant( QString() ) );
    }

    const QString &uniqueId = featPair.second;
    QString md5;
    if ( mDistinctSelect )
    {
      md5 = getMD5( srcFeature );
      if ( existingMD5s.contains( md5 ) )
        continue;
      existingMD5s.insert( md5 );
    }
    else
    {
      if ( uniqueId.isEmpty() )
      {
        // Shouldn't happen on sane datasets.
      }
      else if ( existingUniqueIds.contains( uniqueId ) )
      {
        if ( mRect.isEmpty() )
        {
          QgsDebugMsgLevel( QStringLiteral( "duplicate uniqueId %1" ).arg( uniqueId ), 4 );
        }
        continue;
      }
      else
      {
        existingUniqueIds.insert( uniqueId );
      }
    }

    updatedFeatureList.push_back( featPair );

    //and the attributes
    for ( int i = 0; i < mFields.size(); i++ )
    {
      int idx = dataProviderFields.indexFromName( mMapUserVisibleFieldNameToSpatialiteColumnName[mFields.at( i ).name()] );
      if ( idx >= 0 )
      {
        const QVariant &v = srcFeature.attributes().value( i );
        const QVariant::Type fieldType = dataProviderFields.at( idx ).type();
        if ( v.type() == QVariant::DateTime && !v.isNull() )
          cachedFeature.setAttribute( idx, QVariant( v.toDateTime().toMSecsSinceEpoch() ) );
        else if ( QgsWFSUtils::isCompatibleType( v.type(), fieldType ) )
          cachedFeature.setAttribute( idx, v );
        else
          cachedFeature.setAttribute( idx, QgsVectorDataProvider::convertValue( fieldType, v.toString() ) );
      }
    }

    if ( mDistinctSelect && md5Idx >= 0 )
      cachedFeature.setAttribute( md5Idx, QVariant( md5 ) );

    cachedFeature.setAttribute( uniqueIdIdx, QVariant( uniqueId ) );

    cachedFeature.setAttribute( genCounterIdx, QVariant( genCounter ) );

    // valid
    cachedFeature.setValid( true );
    cachedFeature.setFields( dataProviderFields ); // allow name-based attribute lookups

    featureListToCache.push_back( cachedFeature );
  }

  // In case we would a WFS-T insert, while another thread download features,
  // take a mutex
  {
    QMutexLocker lockerWrite( &mCacheWriteMutex );
    bool cacheOk = mCacheDataProvider->addFeatures( featureListToCache );

    // Update the feature ids of the non-cached feature, i.e. the one that
    // will be notified to the user, from the feature id of the database
    // That way we will always have a consistent feature id, even in case of
    // paging or BBOX request
    Q_ASSERT( featureListToCache.size() == updatedFeatureList.size() );
    for ( int i = 0; i < updatedFeatureList.size(); i++ )
    {
      int resultCode;
      QgsFeatureId dbId( cacheOk ? featureListToCache[i].id() : mTotalFeaturesAttemptedToBeCached + i + 1 );
      QgsFeatureId qgisId;
      const auto &uniqueId( updatedFeatureList[i].second );
      if ( uniqueId.isEmpty() )
      {
        // Degraded case. Won't work properly in reload situations, but we
        // can't do better.
        qgisId = dbId;
      }
      else
      {
        QString errorMsg;

        QString sql = qgs_sqlite3_mprintf( "SELECT qgisId, dbId FROM id_cache WHERE uniqueId = '%q'",
                                           uniqueId.toUtf8().constData() );
        auto stmt = mCacheIdDb.prepare( sql, resultCode );
        Q_ASSERT( resultCode == SQLITE_OK );
        if ( stmt.step() == SQLITE_ROW )
        {
          qgisId = stmt.columnAsInt64( 0 );
          QgsFeatureId oldDbId = stmt.columnAsInt64( 1 );
          if ( dbId != oldDbId )
          {
            sql = qgs_sqlite3_mprintf( "UPDATE id_cache SET dbId = NULL WHERE dbId = %lld",
                                       dbId );
            if ( mCacheIdDb.exec( sql, errorMsg ) != SQLITE_OK )
            {
              QgsMessageLog::logMessage( QObject::tr( "Problem when updating id cache: %1 -> %2" ).arg( sql ).arg( errorMsg ), mComponentTranslated );
            }

            sql = qgs_sqlite3_mprintf( "UPDATE id_cache SET dbId = %lld WHERE uniqueId = '%q'",
                                       dbId,
                                       uniqueId.toUtf8().constData() );
            if ( mCacheIdDb.exec( sql, errorMsg ) != SQLITE_OK )
            {
              QgsMessageLog::logMessage( QObject::tr( "Problem when updating id cache: %1 -> %2" ).arg( sql ).arg( errorMsg ), mComponentTranslated );
            }
          }
        }
        else
        {
          sql = qgs_sqlite3_mprintf( "UPDATE id_cache SET dbId = NULL WHERE dbId = %lld",
                                     dbId );
          if ( mCacheIdDb.exec( sql, errorMsg ) != SQLITE_OK )
          {
            QgsMessageLog::logMessage( QObject::tr( "Problem when updating id cache: %1 -> %2" ).arg( sql ).arg( errorMsg ), mComponentTranslated );
          }

          qgisId = mNextCachedIdQgisId;
          mNextCachedIdQgisId ++;
          sql = qgs_sqlite3_mprintf( "INSERT INTO id_cache (uniqueId, dbId, qgisId) VALUES ('%q', %lld, %lld)",
                                     uniqueId.toUtf8().constData(),
                                     dbId,
                                     qgisId );
          if ( mCacheIdDb.exec( sql, errorMsg ) != SQLITE_OK )
          {
            QgsMessageLog::logMessage( QObject::tr( "Problem when updating id cache: %1 -> %2" ).arg( sql ).arg( errorMsg ), mComponentTranslated );
          }
        }
      }

      updatedFeatureList[i].first.setId( qgisId );
    }

    {
      QMutexLocker locker( &mMutex );
      if ( mRequestLimit != 1 )
      {
        if ( !mFeatureCountExact )
          mFeatureCount += featureListToCache.size();
        mTotalFeaturesAttemptedToBeCached += featureListToCache.size();
        if ( !localComputedExtent.isNull() && mComputedExtent.isNull() && !mTryFetchingOneFeature &&
             !localComputedExtent.intersects( mCapabilityExtent ) )
        {
          QgsMessageLog::logMessage( QObject::tr( "Layer extent reported by the server is not correct. "
                                                  "You may need to zoom again on layer while features are being downloaded" ), mComponentTranslated );
        }
        mComputedExtent = localComputedExtent;
      }
    }
  }

  featureList = updatedFeatureList;

  emitExtentUpdated();

  QgsDebugMsgLevel( QStringLiteral( "end %1" ).arg( featureList.size() ), 4 );

}

const QgsRectangle &QgsBackgroundCachedSharedData::computedExtent() const
{
  QMutexLocker locker( &mMutex );
  return mComputedExtent;
}

//! Called by QgsFeatureDownloaderImpl::run() at the end of the download process.
void QgsBackgroundCachedSharedData::endOfDownload( bool success, long long featureCount,
    bool truncatedResponse,
    bool interrupted,
    const QString &errorMsg )
{
  QMutexLocker locker( &mMutex );

  if ( !success && !interrupted )
  {
    QString errorMsgOut( QObject::tr( "Download of features for layer %1 failed or partially failed: %2. You may attempt reloading the layer with F5" ).arg( layerName(), errorMsg ) );
    pushError( errorMsgOut );
  }

  bool bDownloadLimit = truncatedResponse || ( featureCount >= mMaxFeatures && mMaxFeatures > 0 );

  mDownloadFinished = true;
  if ( success && !mRect.isEmpty() )
  {
    // In the case we requested an extent that includes the extent reported by GetCapabilities response,
    // that we have no filter and we got no features, then it is not unlikely that the capabilities
    // might be wrong. In which case, query one feature so that we got a beginning of extent from
    // which the user will be able to zoom out. This is far from being ideal...
    if ( featureCount == 0 && mRect.contains( mCapabilityExtent ) && !hasServerSideFilter() &&
         supportsFastFeatureCount() && hasGeometry() && !mTryFetchingOneFeature )
    {
      QgsDebugMsg( QStringLiteral( "Capability extent is probably wrong. Starting a new request with one feature limit to get at least one feature" ) );
      mTryFetchingOneFeature = true;
      mComputedExtent = getExtentFromSingleFeatureRequest();
      if ( !mComputedExtent.isNull() &&
           !detectPotentialServerAxisOrderIssueFromSingleFeatureExtent() )
      {
        // Grow the extent by ~ 50 km (completely arbitrary number if you wonder!)
        // so that it is sufficiently zoomed out
        if ( mSourceCrs.mapUnits() == QgsUnitTypes::DistanceMeters )
          mComputedExtent.grow( 50. * 1000. );
        else if ( mSourceCrs.mapUnits() == QgsUnitTypes::DistanceDegrees )
          mComputedExtent.grow( 50. / 110 );
        pushError( QObject::tr( "Layer extent reported by the server is not correct. "
                                "You may need to zoom on layer and then zoom out to see all features" ) );
      }
      mMutex.unlock();
      if ( !mComputedExtent.isNull() )
      {
        emitExtentUpdated();
      }
      mMutex.lock();
      return;
    }

    // Arbitrary threshold to avoid the cache of BBOX to grow out of control.
    // Note: we could be smarter and keep some BBOXes, but the saturation is
    // unlikely to happen in practice, so just clear everything.
    if ( mRegions.size() == 1000000 )
    {
      mRegions.clear();
      mCachedRegions = QgsSpatialIndex();
    }

    if ( mRequestLimit == 0 )
    {
      // In case the download was successful, we will remember this bbox
      // and if the download reached the download limit or not
      QgsFeature f;
      f.setGeometry( QgsGeometry::fromRect( mRect ) );
      QgsFeatureId id = mRegions.size();
      f.setId( id );
      f.initAttributes( 1 );
      f.setAttribute( 0, QVariant( bDownloadLimit ) );
      mRegions.push_back( f );
      mCachedRegions.addFeature( f );
    }
  }

  if ( mRect.isEmpty() && success && !bDownloadLimit && mRequestLimit == 0 && !mFeatureCountExact )
  {
    mFeatureCountExact = true;
    if ( featureCount != mFeatureCount )
    {
      // Shouldn't happen unless there's a bug somewhere or the result returned
      // contains duplicates. Actually happens with
      // http://demo.opengeo.org/geoserver/wfs?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=osm:landcover_line
      // with gml.id=landcover_line.119246130 in particular
      QgsDebugMsg( QStringLiteral( "raw features=%1, unique features=%2" ).
                   arg( featureCount ).arg( mFeatureCount ) );
    }
  }

  if ( bDownloadLimit )
  {
    QString msg( QObject::tr( "%1: The download limit has been reached." ).arg( layerName() ) );
    if ( !mRect.isEmpty() )
      msg += " " + QObject::tr( "Zoom in to fetch all data." );
    else
      msg += " " + QObject::tr( "You may want to check the 'Only request features overlapping the view extent' option to be able to zoom in to fetch all data." );
    QgsMessageLog::logMessage( msg, mComponentTranslated );
  }
}


QSet<QString> QgsBackgroundCachedSharedData::getExistingCachedUniqueIds( const QVector<QgsFeatureUniqueIdPair> &featureList )
{
  // We query the Spatialite cache here, not the persistent id_cache,
  // since we want to know which features in this session we have already
  // downloaded.

  QString expr;
  bool first = true;
  QSet<QString> setExistingUniqueIds;

  QgsFields dataProviderFields = mCacheDataProvider->fields();
  const int uniqueIdIdx = dataProviderFields.indexFromName( QgsBackgroundCachedFeatureIteratorConstants::FIELD_UNIQUE_ID );

  // To avoid excessive memory consumption in expression building, do not
  // query more than 1000 ids at a time.
  for ( int i = 0; i < featureList.size(); i ++ )
  {
    if ( !first )
      expr += ',';
    else
    {
      expr = QgsBackgroundCachedFeatureIteratorConstants::FIELD_UNIQUE_ID + QStringLiteral( " IN (" );
      first = false;
    }
    expr += '\'';
    expr += featureList[i].second;
    expr += '\'';

    if ( ( i > 0 && ( i % 1000 ) == 0 ) || i + 1 == featureList.size() )
    {
      expr += ')';

      QgsFeatureRequest request;
      request.setFilterExpression( expr );

      QgsAttributeList attList;
      attList.append( uniqueIdIdx );
      request.setSubsetOfAttributes( attList );

      QgsFeatureIterator iterUniqueIds( mCacheDataProvider->getFeatures( request ) );
      QgsFeature uniqueIdFeature;
      while ( iterUniqueIds.nextFeature( uniqueIdFeature ) )
      {
        const QVariant &v = uniqueIdFeature.attributes().value( uniqueIdIdx );
        setExistingUniqueIds.insert( v.toString() );
      }

      first = true;
    }

  }

  return setExistingUniqueIds;
}

QSet<QString> QgsBackgroundCachedSharedData::getExistingCachedMD5( const QVector<QgsFeatureUniqueIdPair> &featureList )
{
  QString expr;
  bool first = true;
  QSet<QString> setExistingMD5;

  QgsFields dataProviderFields = mCacheDataProvider->fields();
  const int md5Idx = dataProviderFields.indexFromName( QgsBackgroundCachedFeatureIteratorConstants::FIELD_MD5 );

  // To avoid excessive memory consumption in expression building, do not
  // query more than 1000 ids at a time.
  for ( int i = 0; i < featureList.size(); i ++ )
  {
    if ( !first )
      expr += QLatin1Char( ',' );
    else
    {
      expr = QgsBackgroundCachedFeatureIteratorConstants::FIELD_MD5 + " IN (";
      first = false;
    }
    expr += QLatin1Char( '\'' );
    expr += getMD5( featureList[i].first );
    expr += QLatin1Char( '\'' );

    if ( ( i > 0 && ( i % 1000 ) == 0 ) || i + 1 == featureList.size() )
    {
      expr += QLatin1Char( ')' );

      QgsFeatureRequest request;
      request.setFilterExpression( expr );

      QgsAttributeList attList;
      attList.append( md5Idx );
      request.setSubsetOfAttributes( attList );

      QgsFeatureIterator iterMD5s( mCacheDataProvider->getFeatures( request ) );
      QgsFeature uniqueIdFeature;
      while ( iterMD5s.nextFeature( uniqueIdFeature ) )
      {
        const QVariant &v = uniqueIdFeature.attributes().value( md5Idx );
        setExistingMD5.insert( v.toString() );
      }

      first = true;
    }

  }

  return setExistingMD5;
}

// Used by WFS-T
QString QgsBackgroundCachedSharedData::findUniqueId( QgsFeatureId fid ) const
{
  if ( !mCacheIdDb )
    return QString();

  QString sql = qgs_sqlite3_mprintf( "SELECT uniqueId FROM id_cache WHERE qgisId = %lld", fid );
  int resultCode;
  auto stmt = mCacheIdDb.prepare( sql, resultCode );
  Q_ASSERT( resultCode == SQLITE_OK );
  if ( stmt.step() == SQLITE_ROW )
  {
    return stmt.columnAsText( 0 );
  }
  return QString();
}

QgsFeatureIds QgsBackgroundCachedSharedData::dbIdsFromQgisIds( const QgsFeatureIds &qgisIds ) const
{
  QgsFeatureIds dbIds;
  if ( !mCacheIdDb )
    return dbIds;
  // To avoid excessive memory consumption in expression building, do not
  // query more than 1000 ids at a time.
  bool first = true;
  QString expr;
  int i = 0;
  for ( const auto &qgisId : qgisIds )
  {
    if ( !first )
      expr += ',';
    else
    {
      expr = QStringLiteral( "SELECT dbId FROM id_cache WHERE qgisId IN (" );
      first = false;
    }
    expr += FID_TO_STRING( qgisId );

    if ( ( i > 0 && ( i % 1000 ) == 0 ) || i + 1 == qgisIds.size() )
    {
      expr += ')';

      int resultCode;
      auto stmt = mCacheIdDb.prepare( expr.toUtf8().constData(), resultCode );
      Q_ASSERT( resultCode == SQLITE_OK );
      while ( stmt.step() == SQLITE_ROW )
      {
        dbIds.insert( stmt.columnAsInt64( 0 ) );
      }
      // Should we check that we got a dbId from every qgisId... ?

      first = true;
    }
    i++;
  }
  return dbIds;
}

// Used by WFS-T
bool QgsBackgroundCachedSharedData::deleteFeatures( const QgsFeatureIds &fidlist )
{
  if ( !mCacheIdDb || !mCacheDataProvider )
    return false;

  {
    QMutexLocker locker( &mMutex );
    mFeatureCount -= fidlist.size();
  }

  return mCacheDataProvider->deleteFeatures( dbIdsFromQgisIds( fidlist ) );
}

// Used by WFS-T
bool QgsBackgroundCachedSharedData::changeGeometryValues( const QgsGeometryMap &geometry_map )
{
  if ( !mCacheIdDb || !mCacheDataProvider )
    return false;

  // We need to replace the geometry by its bounding box and issue a attribute
  // values change with the real geometry serialized as hexwkb.

  int idx = mCacheDataProvider->fields().indexFromName( QgsBackgroundCachedFeatureIteratorConstants::FIELD_HEXWKB_GEOM );
  Q_ASSERT( idx >= 0 );

  QgsGeometryMap newGeometryMap;
  QgsChangedAttributesMap newChangedAttrMap;
  for ( QgsGeometryMap::const_iterator iter = geometry_map.constBegin(); iter != geometry_map.constEnd(); ++iter )
  {
    QString sql = qgs_sqlite3_mprintf( "SELECT dbId FROM id_cache WHERE qgisId = %lld", iter.key() );
    int resultCode;
    auto stmt = mCacheIdDb.prepare( sql, resultCode );
    Q_ASSERT( resultCode == SQLITE_OK );
    if ( stmt.step() != SQLITE_ROW )
    {
      // shouldn't happen normally
      QgsDebugMsg( QStringLiteral( "cannot find dbId corresponding to qgisId = %1" ).arg( iter.key() ) );
      continue;
    }
    QgsFeatureId dbId = stmt.columnAsInt64( 0 );

    QByteArray wkb = iter->asWkb();
    if ( !wkb.isEmpty() )
    {
      QgsAttributeMap newAttrMap;
      newAttrMap[idx] = QString( wkb.toHex().data() );
      newChangedAttrMap[ dbId] = newAttrMap;

      QgsGeometry polyBoundingBox = QgsGeometry::fromRect( iter.value().boundingBox() );
      newGeometryMap[ dbId] = polyBoundingBox;
    }
    else
    {
      QgsAttributeMap newAttrMap;
      newAttrMap[idx] = QString();
      newChangedAttrMap[ dbId] = newAttrMap;
      newGeometryMap[ dbId] = QgsGeometry();
    }
  }

  return mCacheDataProvider->changeGeometryValues( newGeometryMap ) &&
         mCacheDataProvider->changeAttributeValues( newChangedAttrMap );
}

// Used by WFS-T
bool QgsBackgroundCachedSharedData::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  if ( !mCacheIdDb || !mCacheDataProvider )
    return false;

  QgsFields dataProviderFields = mCacheDataProvider->fields();
  QgsChangedAttributesMap newMap;
  for ( QgsChangedAttributesMap::const_iterator iter = attr_map.begin(); iter != attr_map.end(); ++iter )
  {
    QString sql = qgs_sqlite3_mprintf( "SELECT dbId FROM id_cache WHERE qgisId = %lld", iter.key() );
    int resultCode;
    auto stmt = mCacheIdDb.prepare( sql, resultCode );
    Q_ASSERT( resultCode == SQLITE_OK );
    if ( stmt.step() != SQLITE_ROW )
    {
      // shouldn't happen normally
      QgsDebugMsg( QStringLiteral( "cannot find dbId corresponding to qgisId = %1" ).arg( iter.key() ) );
      continue;
    }
    QgsFeatureId dbId = stmt.columnAsInt64( 0 );

    const QgsAttributeMap &attrs = iter.value();
    if ( attrs.isEmpty() )
      continue;
    QgsAttributeMap newAttrMap;
    for ( QgsAttributeMap::const_iterator siter = attrs.begin(); siter != attrs.end(); ++siter )
    {
      int idx = dataProviderFields.indexFromName( mMapUserVisibleFieldNameToSpatialiteColumnName[mFields.at( siter.key() ).name()] );
      Q_ASSERT( idx >= 0 );
      if ( siter.value().type() == QVariant::DateTime && !siter.value().isNull() )
        newAttrMap[idx] = QVariant( siter.value().toDateTime().toMSecsSinceEpoch() );
      else
        newAttrMap[idx] = siter.value();
    }
    newMap[dbId] = newAttrMap;
  }

  return mCacheDataProvider->changeAttributeValues( newMap );
}


QString QgsBackgroundCachedSharedData::getMD5( const QgsFeature &f )
{
  const QgsAttributes attrs = f.attributes();
  QCryptographicHash hash( QCryptographicHash::Md5 );
  for ( int i = 0; i < attrs.size(); i++ )
  {
    const QVariant &v = attrs[i];
    hash.addData( QByteArray( ( const char * )&i, sizeof( i ) ) );
    if ( v.isNull() )
    {
      // nothing to do
    }
    else if ( v.type() == QVariant::DateTime )
    {
      qint64 val = v.toDateTime().toMSecsSinceEpoch();
      hash.addData( QByteArray( ( const char * )&val, sizeof( val ) ) );
    }
    else if ( v.type() == QVariant::Int )
    {
      int val = v.toInt();
      hash.addData( QByteArray( ( const char * )&val, sizeof( val ) ) );
    }
    else if ( v.type() == QVariant::LongLong )
    {
      qint64 val = v.toLongLong();
      hash.addData( QByteArray( ( const char * )&val, sizeof( val ) ) );
    }
    else if ( v.type() == QVariant::String )
    {
      hash.addData( v.toByteArray() );
    }
    else if ( v.type() == QVariant::StringList )
    {
      for ( const QString &s : v.toStringList() )
        hash.addData( s.toUtf8() );
    }
  }

  const int attrCount = attrs.size();
  hash.addData( QByteArray( ( const char * )&attrCount, sizeof( attrCount ) ) );
  QgsGeometry geometry = f.geometry();
  if ( !geometry.isNull() )
  {
    hash.addData( geometry.asWkb() );
  }

  return hash.result().toHex();
}

void QgsBackgroundCachedSharedData::setFeatureCount( long long featureCount, bool featureCountExact )
{
  QMutexLocker locker( &mMutex );
  mFeatureCountRequestIssued = true;
  mFeatureCountExact = featureCountExact;
  mFeatureCount = featureCount;
}

long long QgsBackgroundCachedSharedData::getFeatureCount( bool issueRequestIfNeeded )
{
  if ( !mFeatureCountRequestIssued && !mFeatureCountExact &&
       supportsFastFeatureCount() && issueRequestIfNeeded )
  {
    mFeatureCountRequestIssued = true;
    long long featureCount = getFeatureCountFromServer();
    {
      QMutexLocker locker( &mMutex );
      // Check the return value. Might be -1 in case of error, or might be
      // saturated by the server limit, but we may have retrieved more features
      // in the mean time.
      if ( featureCount > mFeatureCount )
      {
        // If the feature count is below or above than the server side limit, we know
        // that it is exact (some server implementation might saturate to the server side limit)
        if ( mServerMaxFeatures > 0 && featureCount != mServerMaxFeatures )
        {
          mFeatureCount = featureCount;
          mFeatureCountExact = true;
        }
        else if ( mServerMaxFeatures <= 0 )
        {
          mFeatureCount = featureCount;
          mFeatureCountExact = true;
        }
      }
    }
  }
  return mFeatureCount;
}

QgsRectangle QgsBackgroundCachedSharedData::consolidatedExtent() const
{
  QMutexLocker locker( &mMutex );

  // Some servers return completely buggy extent in their capabilities response
  // so mix it with the extent actually got from the downloaded features
  QgsRectangle l_computedExtent( mComputedExtent );
  QgsDebugMsgLevel( QStringLiteral( "mComputedExtent: " ) + mComputedExtent.toString(), 4 );
  QgsDebugMsgLevel( QStringLiteral( "mCapabilityExtent: " ) + mCapabilityExtent.toString(), 4 );

  // If we didn't get any feature, then return capabilities extent.
  if ( l_computedExtent.isNull() )
    return mCapabilityExtent;

  // If the capabilities extent is completely off from the features, then
  // use feature extent.
  // Case of standplaats layer of http://geodata.nationaalgeoregister.nl/bag/wfs
  if ( !l_computedExtent.intersects( mCapabilityExtent ) )
    return l_computedExtent;

  if ( downloadFinished() )
  {
    return l_computedExtent;
  }

  l_computedExtent.combineExtentWith( mCapabilityExtent );
  return l_computedExtent;
}
