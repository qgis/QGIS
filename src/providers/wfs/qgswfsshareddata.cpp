/***************************************************************************
    qgswfsshareddata.cpp
    ---------------------
    begin                : March 2016
    copyright            : (C) 2016 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <cmath> // M_PI

#include "qgswfsconstants.h"
#include "qgswfsshareddata.h"
#include "qgswfsutils.h"

#include "qgsogcutils.h"
#include "qgsexpression.h"
#include "qgsmessagelog.h"
#include "qgsspatialindex.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorfilewriter.h"
#include "qgsproviderregistry.h"
#include "qgslogger.h"
#include "qgsspatialiteutils.h"

#include <cpl_vsi.h>
#include <cpl_conv.h>
#include <ogr_api.h>

#include <sqlite3.h>

QgsWFSSharedData::QgsWFSSharedData( const QString &uri )
  : mURI( uri )
  , mSourceCRS( 0 )
  , mMaxFeatures( 0 )
  , mPageSize( 0 )
  , mRequestLimit( 0 )
  , mHideProgressDialog( mURI.hideDownloadProgressDialog() )
  , mDistinctSelect( false )
  , mHasWarnedAboutMissingFeatureId( false )
  , mGetFeatureEPSGDotHonoursEPSGOrder( false )
  , mDownloadFinished( false )
  , mGenCounter( 0 )
  , mFeatureCount( 0 )
  , mFeatureCountExact( false )
  , mGetFeatureHitsIssued( false )
  , mTotalFeaturesAttemptedToBeCached( 0 )
  , mTryFetchingOneFeature( false )
{
  // Needed because used by a signal
  qRegisterMetaType< QVector<QgsWFSFeatureGmlIdPair> >( "QVector<QgsWFSFeatureGmlIdPair>" );
}

QgsWFSSharedData::~QgsWFSSharedData()
{
  QgsDebugMsgLevel( QStringLiteral( "~QgsWFSSharedData()" ), 4 );

  invalidateCache();

  mCacheIdDb.reset();
  if ( !mCacheIdDbname.isEmpty() )
  {
    QFile::remove( mCacheIdDbname );
    QFile::remove( mCacheIdDbname + "-wal" );
    QFile::remove( mCacheIdDbname + "-shm" );
    QgsWFSUtils::releaseCacheDirectory();
    mCacheIdDbname.clear();
  }
}

QString QgsWFSSharedData::srsName() const
{
  QString srsName;
  if ( !mSourceCRS.authid().isEmpty() )
  {
    if ( mWFSVersion.startsWith( QLatin1String( "1.0" ) ) ||
         !mSourceCRS.authid().startsWith( QLatin1String( "EPSG:" ) ) ||
         // For servers like Geomedia that advertize EPSG:XXXX in capabilities even in WFS 1.1 or 2.0
         mCaps.useEPSGColumnFormat )
    {
      srsName = mSourceCRS.authid();
    }
    else
    {
      QStringList list = mSourceCRS.authid().split( ':' );
      srsName = QStringLiteral( "urn:ogc:def:crs:EPSG::%1" ).arg( list.last() );
    }
  }
  return srsName;
}

bool QgsWFSSharedData::computeFilter( QString &errorMsg )
{
  errorMsg.clear();
  mWFSFilter.clear();
  mSortBy.clear();

  QgsOgcUtils::GMLVersion gmlVersion;
  QgsOgcUtils::FilterVersion filterVersion;
  bool honourAxisOrientation = false;
  if ( mWFSVersion.startsWith( QLatin1String( "1.0" ) ) )
  {
    gmlVersion = QgsOgcUtils::GML_2_1_2;
    filterVersion = QgsOgcUtils::FILTER_OGC_1_0;
  }
  else if ( mWFSVersion.startsWith( QLatin1String( "1.1" ) ) )
  {
    honourAxisOrientation = !mURI.ignoreAxisOrientation();
    gmlVersion = QgsOgcUtils::GML_3_1_0;
    filterVersion = QgsOgcUtils::FILTER_OGC_1_1;
  }
  else
  {
    honourAxisOrientation = !mURI.ignoreAxisOrientation();
    gmlVersion = QgsOgcUtils::GML_3_2_1;
    filterVersion = QgsOgcUtils::FILTER_FES_2_0;
  }

  if ( !mURI.sql().isEmpty() )
  {
    QgsSQLStatement sql( mURI.sql() );

    const QgsSQLStatement::NodeSelect *select = dynamic_cast<const QgsSQLStatement::NodeSelect *>( sql.rootNode() );
    if ( !select )
    {
      // Makes Coverity happy, but cannot happen in practice
      QgsDebugMsg( QStringLiteral( "should not happen" ) );
      return false;
    }
    QList<QgsSQLStatement::NodeColumnSorted *> orderBy = select->orderBy();
    Q_FOREACH ( QgsSQLStatement::NodeColumnSorted *columnSorted, orderBy )
    {
      if ( !mSortBy.isEmpty() )
        mSortBy += QLatin1String( "," );
      mSortBy += columnSorted->column()->name();
      if ( !columnSorted->ascending() )
      {
        if ( mWFSVersion.startsWith( QLatin1String( "2.0" ) ) )
          mSortBy += QLatin1String( " DESC" );
        else
          mSortBy += QLatin1String( " D" );
      }
    }

    QDomDocument filterDoc;
    QDomElement filterElem = QgsOgcUtils::SQLStatementToOgcFilter(
                               sql, filterDoc, gmlVersion, filterVersion, mLayerPropertiesList,
                               honourAxisOrientation, mURI.invertAxisOrientation(),
                               mCaps.mapUnprefixedTypenameToPrefixedTypename,
                               &errorMsg );
    if ( !errorMsg.isEmpty() )
    {
      errorMsg = tr( "SQL statement to OGC Filter error: " ) + errorMsg;
      return false;
    }
    if ( !filterElem.isNull() )
    {
      filterDoc.appendChild( filterElem );
      mWFSFilter = filterDoc.toString();
    }
  }
  else
  {
    QString filter( mURI.filter() );
    if ( !filter.isEmpty() )
    {
      //test if filterString is already an OGC filter xml
      QDomDocument filterDoc;
      if ( filterDoc.setContent( filter ) )
      {
        mWFSFilter = filter;
      }
      else
      {
        //if not, if must be a QGIS expression
        QgsExpression filterExpression( filter );

        QDomElement filterElem = QgsOgcUtils::expressionToOgcFilter(
                                   filterExpression, filterDoc, gmlVersion, filterVersion, mGeometryAttribute,
                                   srsName(), honourAxisOrientation, mURI.invertAxisOrientation(),
                                   &errorMsg );

        if ( !errorMsg.isEmpty() )
        {
          errorMsg = tr( "Expression to OGC Filter error: " ) + errorMsg;
          return false;
        }
        if ( !filterElem.isNull() )
        {
          filterDoc.appendChild( filterElem );
          mWFSFilter = filterDoc.toString();
        }
      }
    }
  }

  return true;
}

// We have an issue with GDAL 1.10 and older that is using spatialite_init() which is
// incompatible with how the QGIS SpatiaLite provider works.
// The symptom of the issue is the error message
// 'Unable to Initialize SpatiaLite Metadata: no such function: InitSpatialMetadata'
// So in that case we must use only QGIS functions to avoid the conflict
// The difference is that in the QGIS way we have to create the template database
// on disk, which is a slightly bit slower. But due to later caching, this is
// not so a big deal.
#define USE_OGR_FOR_DB_CREATION

static QString quotedIdentifier( QString id )
{
  id.replace( '\"', QLatin1String( "\"\"" ) );
  return id.prepend( '\"' ).append( '\"' );
}

bool QgsWFSSharedData::createCache()
{
  Q_ASSERT( mCacheDbname.isEmpty() );

  static QAtomicInt sTmpCounter = 0;
  int tmpCounter = ++sTmpCounter;
  QString cacheDirectory( QgsWFSUtils::acquireCacheDirectory() );
  mCacheDbname = QDir( cacheDirectory ).filePath( QStringLiteral( "wfs_cache_%1.sqlite" ).arg( tmpCounter ) );
  Q_ASSERT( !QFile::exists( mCacheDbname ) );

  QgsFields cacheFields;
  for ( const QgsField &field : qgis::as_const( mFields ) )
  {
    QVariant::Type type = field.type();
    // Map DateTime to int64 milliseconds from epoch
    if ( type == QVariant::DateTime )
    {
      // Note: this is just a wish. If GDAL < 2, QgsVectorFileWriter will actually map
      // it to a String
      type = QVariant::LongLong;
    }
    cacheFields.append( QgsField( field.name(), type, field.typeName() ) );
  }
  // Add some field for our internal use
  cacheFields.append( QgsField( QgsWFSConstants::FIELD_GEN_COUNTER, QVariant::Int, QStringLiteral( "int" ) ) );
  cacheFields.append( QgsField( QgsWFSConstants::FIELD_GMLID, QVariant::String, QStringLiteral( "string" ) ) );
  cacheFields.append( QgsField( QgsWFSConstants::FIELD_HEXWKB_GEOM, QVariant::String, QStringLiteral( "string" ) ) );
  if ( mDistinctSelect )
    cacheFields.append( QgsField( QgsWFSConstants::FIELD_MD5, QVariant::String, QStringLiteral( "string" ) ) );

  bool ogrWaySuccessful = false;
  QString fidName( QStringLiteral( "__ogc_fid" ) );
  QString geometryFieldname( QStringLiteral( "__spatialite_geometry" ) );
#ifdef USE_OGR_FOR_DB_CREATION
  // Only GDAL >= 2.0 can use an alternate geometry or FID field name
  // but QgsVectorFileWriter will refuse anyway to create a ogc_fid, so we will
  // do it manually
  bool useReservedNames = cacheFields.lookupField( QStringLiteral( "ogc_fid" ) ) >= 0;
  if ( !useReservedNames )
  {
    // Creating a SpatiaLite database can be quite slow on some file systems
    // so we create a GDAL in-memory file, and then copy it on
    // the file system.
    QString vsimemFilename;
    QStringList datasourceOptions;
    QStringList layerOptions;
    datasourceOptions.push_back( QStringLiteral( "INIT_WITH_EPSG=NO" ) );
    layerOptions.push_back( QStringLiteral( "LAUNDER=NO" ) ); // to get exact matches for field names, especially regarding case
    layerOptions.push_back( QStringLiteral( "FID=__ogc_fid" ) );
    layerOptions.push_back( QStringLiteral( "GEOMETRY_NAME=__spatialite_geometry" ) );
    vsimemFilename.sprintf( "/vsimem/qgis_wfs_cache_template_%p/features.sqlite", this );
    mCacheTablename = CPLGetBasename( vsimemFilename.toStdString().c_str() );
    VSIUnlink( vsimemFilename.toStdString().c_str() );
    std::unique_ptr< QgsVectorFileWriter > writer = qgis::make_unique< QgsVectorFileWriter >( vsimemFilename, QString(),
        cacheFields, QgsWkbTypes::Polygon, QgsCoordinateReferenceSystem(), QStringLiteral( "SpatiaLite" ), datasourceOptions, layerOptions );
    if ( writer->hasError() == QgsVectorFileWriter::NoError )
    {
      writer.reset();

      // Copy the temporary database back to disk
      vsi_l_offset nLength = 0;
      GByte *pabyData = VSIGetMemFileBuffer( vsimemFilename.toStdString().c_str(), &nLength, TRUE );
      Q_ASSERT( !QFile::exists( mCacheDbname ) );
      VSILFILE *fp = VSIFOpenL( mCacheDbname.toStdString().c_str(), "wb " );
      if ( fp )
      {
        VSIFWriteL( pabyData, 1, nLength, fp );
        VSIFCloseL( fp );
        CPLFree( pabyData );
      }
      else
      {
        CPLFree( pabyData );
        QgsMessageLog::logMessage( tr( "Cannot create temporary SpatiaLite cache" ), tr( "WFS" ) );
        return false;
      }

      ogrWaySuccessful = true;
    }
    else
    {
      // Be tolerant on failures. Some (Windows) GDAL >= 1.11 builds may
      // not define SPATIALITE_412_OR_LATER, and thus the call to
      // spatialite_init() may cause failures, which will require using the
      // slower method
      writer.reset();
      VSIUnlink( vsimemFilename.toStdString().c_str() );
    }
  }
#endif
  if ( !ogrWaySuccessful )
  {
    static QMutex sMutexDBnameCreation;
    static QByteArray sCachedDBTemplate;
    QMutexLocker mutexDBnameCreationHolder( &sMutexDBnameCreation );
    if ( sCachedDBTemplate.size() == 0 )
    {
      // Create a template SpatiaLite DB
      QTemporaryFile tempFile;
      tempFile.open();
      tempFile.setAutoRemove( false );
      tempFile.close();
      QString spatialite_lib = QgsProviderRegistry::instance()->library( QStringLiteral( "spatialite" ) );
      QLibrary *myLib = new QLibrary( spatialite_lib );
      bool loaded = myLib->load();
      bool created = false;
      if ( loaded )
      {
        QgsDebugMsgLevel( QStringLiteral( "SpatiaLite provider loaded" ), 4 );

        typedef bool ( *createDbProc )( const QString &, QString & );
        createDbProc createDbPtr = ( createDbProc ) cast_to_fptr( myLib->resolve( "createDb" ) );
        if ( createDbPtr )
        {
          QString errCause;
          created = createDbPtr( tempFile.fileName(), errCause );
        }
      }
      delete myLib;
      if ( !created )
      {
        QgsMessageLog::logMessage( tr( "Cannot create temporary SpatiaLite cache" ), tr( "WFS" ) );
        return false;
      }

      // Ingest it in a buffer
      QFile file( tempFile.fileName() );
      if ( file.open( QIODevice::ReadOnly ) )
        sCachedDBTemplate = file.readAll();
      file.close();
      QFile::remove( tempFile.fileName() );
    }

    // Copy the in-memory template SpatiaLite DB into the target DB
    Q_ASSERT( !QFile::exists( mCacheDbname ) );
    QFile dbFile( mCacheDbname );
    if ( !dbFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    {
      QgsMessageLog::logMessage( tr( "Cannot create temporary SpatiaLite cache" ), tr( "WFS" ) );
      return false;
    }
    if ( dbFile.write( sCachedDBTemplate ) < 0 )
    {
      QgsMessageLog::logMessage( tr( "Cannot create temporary SpatiaLite cache" ), tr( "WFS" ) );
      return false;
    }
  }

  spatialite_database_unique_ptr database;
  bool ret = true;
  int rc = database.open( mCacheDbname );
  if ( rc == SQLITE_OK )
  {
    QString sql;

    ( void )sqlite3_exec( database.get(), "PRAGMA synchronous=OFF", nullptr, nullptr, nullptr );
    // WAL is needed to avoid reader to block writers
    ( void )sqlite3_exec( database.get(), "PRAGMA journal_mode=WAL", nullptr, nullptr, nullptr );

    ( void )sqlite3_exec( database.get(), "BEGIN", nullptr, nullptr, nullptr );

    if ( !ogrWaySuccessful )
    {
      mCacheTablename = QStringLiteral( "features" );
      sql = QStringLiteral( "CREATE TABLE %1 (%2 INTEGER PRIMARY KEY" ).arg( mCacheTablename, fidName );
      for ( const QgsField &field : qgis::as_const( cacheFields ) )
      {
        QString type( QStringLiteral( "VARCHAR" ) );
        if ( field.type() == QVariant::Int )
          type = QStringLiteral( "INTEGER" );
        else if ( field.type() == QVariant::LongLong )
          type = QStringLiteral( "BIGINT" );
        else if ( field.type() == QVariant::Double )
          type = QStringLiteral( "REAL" );
        sql += QStringLiteral( ", %1 %2" ).arg( quotedIdentifier( field.name() ), type );
      }
      sql += QLatin1String( ")" );
      rc = sqlite3_exec( database.get(), sql.toUtf8(), nullptr, nullptr, nullptr );
      if ( rc != SQLITE_OK )
      {
        QgsDebugMsg( QStringLiteral( "%1 failed" ).arg( sql ) );
        ret = false;
      }

      sql = QStringLiteral( "SELECT AddGeometryColumn('%1','%2',0,'POLYGON',2)" ).arg( mCacheTablename, geometryFieldname );
      rc = sqlite3_exec( database.get(), sql.toUtf8(), nullptr, nullptr, nullptr );
      if ( rc != SQLITE_OK )
      {
        QgsDebugMsg( QStringLiteral( "%1 failed" ).arg( sql ) );
        ret = false;
      }

      sql = QStringLiteral( "SELECT CreateSpatialIndex('%1','%2')" ).arg( mCacheTablename, geometryFieldname );
      rc = sqlite3_exec( database.get(), sql.toUtf8(), nullptr, nullptr, nullptr );
      if ( rc != SQLITE_OK )
      {
        QgsDebugMsg( QStringLiteral( "%1 failed" ).arg( sql ) );
        ret = false;
      }
    }

    // We need an index on the gmlid, since we will check for duplicates, particularly
    // useful in the case we do overlapping BBOX requests
    sql = QStringLiteral( "CREATE INDEX idx_%2 ON %1(%2)" ).arg( mCacheTablename, QgsWFSConstants::FIELD_GMLID );
    rc = sqlite3_exec( database.get(), sql.toUtf8(), nullptr, nullptr, nullptr );
    if ( rc != SQLITE_OK )
    {
      QgsDebugMsg( QStringLiteral( "%1 failed" ).arg( sql ) );
      ret = false;
    }

    if ( mDistinctSelect )
    {
      sql = QStringLiteral( "CREATE INDEX idx_%2 ON %1(%2)" ).arg( mCacheTablename, QgsWFSConstants::FIELD_MD5 );
      rc = sqlite3_exec( database.get(), sql.toUtf8(), nullptr, nullptr, nullptr );
      if ( rc != SQLITE_OK )
      {
        QgsDebugMsg( QStringLiteral( "%1 failed" ).arg( sql ) );
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
    QgsMessageLog::logMessage( tr( "Cannot create temporary SpatiaLite cache" ), tr( "WFS" ) );
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
  mCacheDataProvider = ( QgsVectorDataProvider * )( QgsProviderRegistry::instance()->createProvider(
                         QStringLiteral( "spatialite" ), dsURI.uri(), providerOptions ) );
  if ( mCacheDataProvider && !mCacheDataProvider->isValid() )
  {
    delete mCacheDataProvider;
    mCacheDataProvider = nullptr;
  }
  if ( !mCacheDataProvider )
  {
    QgsMessageLog::logMessage( tr( "Cannot connect to temporary SpatiaLite cache" ), tr( "WFS" ) );
    return false;
  }

  // The id_cache should be generated once for the lifetime of QgsWFSConstants
  // to ensure consistency of the ids returned to the user.
  if ( mCacheIdDbname.isEmpty() )
  {
    mCacheIdDbname = QDir( cacheDirectory ).filePath( QStringLiteral( "wfs_id_cache_%1.sqlite" ).arg( tmpCounter ) );
    Q_ASSERT( !QFile::exists( mCacheIdDbname ) );
    if ( mCacheIdDb.open( mCacheIdDbname ) != SQLITE_OK )
    {
      QgsMessageLog::logMessage( tr( "Cannot create temporary id cache" ), tr( "WFS" ) );
      return false;
    }
    QString errorMsg;
    bool ok = mCacheIdDb.exec( QStringLiteral( "PRAGMA synchronous=OFF" ), errorMsg ) == SQLITE_OK;
    // WAL is needed to avoid reader to block writers
    ok &= mCacheIdDb.exec( QStringLiteral( "PRAGMA journal_mode=WAL" ), errorMsg ) == SQLITE_OK;
    // gmlid is the gmlid or fid attribute coming from the GML GetFeature response
    // qgisId is the feature id of the features returned to QGIS. That one should remain the same for a given gmlid even after a layer reload
    // dbId is the feature id of the Spatialite feature in mCacheDataProvider. It might change for a given gmlid after a layer reload
    ok &= mCacheIdDb.exec( QStringLiteral( "CREATE TABLE id_cache(gmlid TEXT, dbId INTEGER, qgisId INTEGER)" ), errorMsg ) == SQLITE_OK;
    ok &= mCacheIdDb.exec( QStringLiteral( "CREATE INDEX idx_gmlid ON id_cache(gmlid)" ), errorMsg ) == SQLITE_OK;
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

int QgsWFSSharedData::registerToCache( QgsWFSFeatureIterator *iterator, int limit, const QgsRectangle &rect )
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
    Q_FOREACH ( QgsFeatureId id, intersectingRequests )
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
  else if ( !( mWFSVersion.startsWith( QLatin1String( "1.0" ) ) ) && limit <= 0 && mRequestLimit > 0 )
  {
    newDownloadNeeded = true;
  }

  if ( newDownloadNeeded || !mDownloader )
  {
    mRect = rect;
    mRequestLimit = ( limit > 0 && !( mWFSVersion.startsWith( QLatin1String( "1.0" ) ) ) ) ? limit : 0;
    // to prevent deadlock when waiting the end of the downloader thread that will try to take the mutex in serializeFeatures()
    mMutex.unlock();
    delete mDownloader;
    mMutex.lock();
    mDownloadFinished = false;
    mComputedExtent = QgsRectangle();
    mDownloader = new QgsWFSThreadedFeatureDownloader( this );
    mDownloader->startAndWait();
  }
  if ( mDownloadFinished )
    return -1;

  // Under the mutex to ensure that we will not miss features
  iterator->connectSignals( mDownloader->downloader() );

  return mGenCounter ++;
}

int QgsWFSSharedData::getUpdatedCounter()
{
  QMutexLocker locker( &mMutex );
  if ( mDownloadFinished )
    return mGenCounter;
  return mGenCounter ++;
}

QSet<QString> QgsWFSSharedData::getExistingCachedGmlIds( const QVector<QgsWFSFeatureGmlIdPair> &featureList )
{
  // We query the Spatialite cache here, not the persistent id_cache,
  // since we want to know which features in this session we have already
  // downloaded.

  QString expr;
  bool first = true;
  QSet<QString> setExistingGmlIds;

  QgsFields dataProviderFields = mCacheDataProvider->fields();
  const int gmlidIdx = dataProviderFields.indexFromName( QgsWFSConstants::FIELD_GMLID );

  // To avoid excessive memory consumption in expression building, do not
  // query more than 1000 ids at a time.
  for ( int i = 0; i < featureList.size(); i ++ )
  {
    if ( !first )
      expr += QLatin1String( "," );
    else
    {
      expr = QgsWFSConstants::FIELD_GMLID + " IN (";
      first = false;
    }
    expr += QLatin1String( "'" );
    expr += featureList[i].second;
    expr += QLatin1String( "'" );

    if ( ( i > 0 && ( i % 1000 ) == 0 ) || i + 1 == featureList.size() )
    {
      expr += QLatin1String( ")" );

      QgsFeatureRequest request;
      request.setFilterExpression( expr );

      QgsAttributeList attList;
      attList.append( gmlidIdx );
      request.setSubsetOfAttributes( attList );

      QgsFeatureIterator iterGmlIds( mCacheDataProvider->getFeatures( request ) );
      QgsFeature gmlidFeature;
      while ( iterGmlIds.nextFeature( gmlidFeature ) )
      {
        const QVariant &v = gmlidFeature.attributes().value( gmlidIdx );
        setExistingGmlIds.insert( v.toString() );
      }

      first = true;
    }

  }

  return setExistingGmlIds;
}

QSet<QString> QgsWFSSharedData::getExistingCachedMD5( const QVector<QgsWFSFeatureGmlIdPair> &featureList )
{
  QString expr;
  bool first = true;
  QSet<QString> setExistingMD5;

  QgsFields dataProviderFields = mCacheDataProvider->fields();
  const int md5Idx = dataProviderFields.indexFromName( QgsWFSConstants::FIELD_MD5 );

  // To avoid excessive memory consumption in expression building, do not
  // query more than 1000 ids at a time.
  for ( int i = 0; i < featureList.size(); i ++ )
  {
    if ( !first )
      expr += QLatin1String( "," );
    else
    {
      expr = QgsWFSConstants::FIELD_MD5 + " IN (";
      first = false;
    }
    expr += QLatin1String( "'" );
    expr += QgsWFSUtils::getMD5( featureList[i].first );
    expr += QLatin1String( "'" );

    if ( ( i > 0 && ( i % 1000 ) == 0 ) || i + 1 == featureList.size() )
    {
      expr += QLatin1String( ")" );

      QgsFeatureRequest request;
      request.setFilterExpression( expr );

      QgsAttributeList attList;
      attList.append( md5Idx );
      request.setSubsetOfAttributes( attList );

      QgsFeatureIterator iterMD5s( mCacheDataProvider->getFeatures( request ) );
      QgsFeature gmlidFeature;
      while ( iterMD5s.nextFeature( gmlidFeature ) )
      {
        const QVariant &v = gmlidFeature.attributes().value( md5Idx );
        setExistingMD5.insert( v.toString() );
      }

      first = true;
    }

  }

  return setExistingMD5;
}

// Used by WFS-T
QString QgsWFSSharedData::findGmlId( QgsFeatureId fid )
{
  if ( !mCacheIdDb )
    return QString();

  auto sql = QgsSqlite3Mprintf( "SELECT gmlid FROM id_cache WHERE qgisId = %lld", fid );
  int resultCode;
  auto stmt = mCacheIdDb.prepare( sql, resultCode );
  Q_ASSERT( resultCode == SQLITE_OK );
  if ( stmt.step() == SQLITE_ROW )
  {
    return stmt.columnAsText( 0 );
  }
  return QString();
}

QgsFeatureIds QgsWFSSharedData::dbIdsFromQgisIds( const QgsFeatureIds &qgisIds )
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
bool QgsWFSSharedData::deleteFeatures( const QgsFeatureIds &fidlist )
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
bool QgsWFSSharedData::changeGeometryValues( const QgsGeometryMap &geometry_map )
{
  if ( !mCacheIdDb || !mCacheDataProvider )
    return false;

  // We need to replace the geometry by its bounding box and issue a attribute
  // values change with the real geometry serialized as hexwkb.

  int idx = mCacheDataProvider->fields().indexFromName( QgsWFSConstants::FIELD_HEXWKB_GEOM );
  Q_ASSERT( idx >= 0 );

  QgsGeometryMap newGeometryMap;
  QgsChangedAttributesMap newChangedAttrMap;
  for ( QgsGeometryMap::const_iterator iter = geometry_map.constBegin(); iter != geometry_map.constEnd(); ++iter )
  {
    auto sql = QgsSqlite3Mprintf( "SELECT dbId FROM id_cache WHERE qgisId = %lld", iter.key() );
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
bool QgsWFSSharedData::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  if ( !mCacheIdDb || !mCacheDataProvider )
    return false;

  QgsFields dataProviderFields = mCacheDataProvider->fields();
  QgsChangedAttributesMap newMap;
  for ( QgsChangedAttributesMap::const_iterator iter = attr_map.begin(); iter != attr_map.end(); ++iter )
  {
    auto sql = QgsSqlite3Mprintf( "SELECT dbId FROM id_cache WHERE qgisId = %lld", iter.key() );
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
      int idx = dataProviderFields.indexFromName( mFields.at( siter.key() ).name() );
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

void QgsWFSSharedData::serializeFeatures( QVector<QgsWFSFeatureGmlIdPair> &featureList )
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
  int gmlidIdx = dataProviderFields.indexFromName( QgsWFSConstants::FIELD_GMLID );
  Q_ASSERT( gmlidIdx >= 0 );
  int genCounterIdx = dataProviderFields.indexFromName( QgsWFSConstants::FIELD_GEN_COUNTER );
  Q_ASSERT( genCounterIdx >= 0 );
  int hexwkbGeomIdx = dataProviderFields.indexFromName( QgsWFSConstants::FIELD_HEXWKB_GEOM );
  Q_ASSERT( hexwkbGeomIdx >= 0 );
  int md5Idx = ( mDistinctSelect ) ? dataProviderFields.indexFromName( QgsWFSConstants::FIELD_MD5 ) : -1;

  QSet<QString> existingGmlIds;
  QSet<QString> existingMD5s;
  if ( mDistinctSelect )
    existingMD5s = getExistingCachedMD5( featureList );
  else
    existingGmlIds = getExistingCachedGmlIds( featureList );
  QVector<QgsWFSFeatureGmlIdPair> updatedFeatureList;

  QgsRectangle localComputedExtent( mComputedExtent );
  Q_FOREACH ( const QgsWFSFeatureGmlIdPair &featPair, featureList )
  {
    const QgsFeature &gmlFeature = featPair.first;

    QgsFeature cachedFeature;
    cachedFeature.initAttributes( dataProviderFields.size() );

    // copy the geometry
    // Do this now to update localComputedExtent, even if we skip the feature
    // afterwards as being already downloaded.
    QgsGeometry geometry = gmlFeature.geometry();
    if ( !mGeometryAttribute.isEmpty() && !geometry.isNull() )
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

    const QString &gmlId = featPair.second;
    QString md5;
    if ( mDistinctSelect )
    {
      md5 = QgsWFSUtils::getMD5( gmlFeature );
      if ( existingMD5s.contains( md5 ) )
        continue;
      existingMD5s.insert( md5 );
    }
    else
    {
      if ( gmlId.isEmpty() )
      {
        // Shouldn't happen on sane datasets.
      }
      else if ( existingGmlIds.contains( gmlId ) )
      {
        if ( mRect.isEmpty() )
        {
          QgsDebugMsgLevel( QStringLiteral( "duplicate gmlId %1" ).arg( gmlId ), 4 );
        }
        continue;
      }
      else
      {
        existingGmlIds.insert( gmlId );
      }
    }

    updatedFeatureList.push_back( featPair );

    //and the attributes
    for ( int i = 0; i < mFields.size(); i++ )
    {
      int idx = dataProviderFields.indexFromName( mFields.at( i ).name() );
      if ( idx >= 0 )
      {
        const QVariant &v = gmlFeature.attributes().value( i );
        if ( v.type() == QVariant::DateTime && !v.isNull() )
          cachedFeature.setAttribute( idx, QVariant( v.toDateTime().toMSecsSinceEpoch() ) );
        else if ( v.type() != dataProviderFields.at( idx ).type() )
          cachedFeature.setAttribute( idx, QgsVectorDataProvider::convertValue( dataProviderFields.at( idx ).type(), v.toString() ) );
        else
          cachedFeature.setAttribute( idx, v );
      }
    }

    if ( mDistinctSelect && md5Idx >= 0 )
      cachedFeature.setAttribute( md5Idx, QVariant( md5 ) );

    cachedFeature.setAttribute( gmlidIdx, QVariant( gmlId ) );

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
      const auto &gmlId( updatedFeatureList[i].second );
      if ( gmlId.isEmpty() )
      {
        // Degraded case. Won't work properly in reload situations, but we
        // can't do better.
        qgisId = dbId;
      }
      else
      {
        auto sql = QgsSqlite3Mprintf( "SELECT qgisId, dbId FROM id_cache WHERE gmlid = '%q'",
                                      gmlId.toUtf8().constData() );
        auto stmt = mCacheIdDb.prepare( sql, resultCode );
        Q_ASSERT( resultCode == SQLITE_OK );
        if ( stmt.step() == SQLITE_ROW )
        {
          qgisId = stmt.columnAsInt64( 0 );
          QgsFeatureId oldDbId = stmt.columnAsInt64( 1 );
          if ( dbId != oldDbId )
          {
            sql = QgsSqlite3Mprintf( "UPDATE id_cache SET dbId = %lld WHERE gmlid = '%q'",
                                     dbId,
                                     gmlId.toUtf8().constData() );
            //QgsDebugMsg( QStringLiteral( "%1" ).arg( sql ) );
            QString errorMsg;
            if ( mCacheIdDb.exec( sql, errorMsg ) != SQLITE_OK )
            {
              QgsMessageLog::logMessage( tr( "Problem when updating WFS id cache: %1 -> %2" ).arg( sql ).arg( errorMsg ), tr( "WFS" ) );
            }
          }
        }
        else
        {
          qgisId = mNextCachedIdQgisId;
          mNextCachedIdQgisId ++;
          sql = QgsSqlite3Mprintf( "INSERT INTO id_cache (gmlid, dbId, qgisId) VALUES ('%q', %lld, %lld)",
                                   gmlId.toUtf8().constData(),
                                   dbId,
                                   qgisId );
          //QgsDebugMsg( QStringLiteral( "%1" ).arg( sql ) );
          QString errorMsg;
          if ( mCacheIdDb.exec( sql, errorMsg ) != SQLITE_OK )
          {
            QgsMessageLog::logMessage( tr( "Problem when updating WFS id cache: %1 -> %2" ).arg( sql ).arg( errorMsg ), tr( "WFS" ) );
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
          QgsMessageLog::logMessage( tr( "Layer extent reported by the server is not correct. "
                                         "You may need to zoom again on layer while features are being downloaded" ), tr( "WFS" ) );
        }
        mComputedExtent = localComputedExtent;
      }
    }
  }

  featureList = updatedFeatureList;

  emit extentUpdated();

  QgsDebugMsgLevel( QStringLiteral( "end %1" ).arg( featureList.size() ), 4 );

}

QgsRectangle QgsWFSSharedData::computedExtent()
{
  QMutexLocker locker( &mMutex );
  return mComputedExtent;
}

void QgsWFSSharedData::pushError( const QString &errorMsg )
{
  QgsMessageLog::logMessage( errorMsg, tr( "WFS" ) );
  emit raiseError( errorMsg );
}

//! Called by QgsWFSFeatureDownloader::run() at the end of the download process.
void QgsWFSSharedData::endOfDownload( bool success, int featureCount,
                                      bool truncatedResponse,
                                      bool interrupted,
                                      const QString &errorMsg )
{
  QMutexLocker locker( &mMutex );

  if ( !success && !interrupted )
  {
    QString errorMsgOut( tr( "Download of features for layer %1 failed or partially failed: %2. You may attempt reloading the layer with F5" ).arg( mURI.typeName(), errorMsg ) );
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
    if ( featureCount == 0 && mRect.contains( mCapabilityExtent ) && mWFSFilter.isEmpty() &&
         mCaps.supportsHits && !mGeometryAttribute.isEmpty() && !mTryFetchingOneFeature )
    {
      QgsDebugMsg( QStringLiteral( "Capability extent is probably wrong. Starting a new request with one feature limit to get at least one feature" ) );
      mTryFetchingOneFeature = true;
      QgsWFSSingleFeatureRequest request( this );
      mComputedExtent = request.getExtent();
      if ( !mComputedExtent.isNull() )
      {
        // Grow the extent by ~ 50 km (completely arbitrary number if you wonder!)
        // so that it is sufficiently zoomed out
        if ( mSourceCRS.mapUnits() == QgsUnitTypes::DistanceMeters )
          mComputedExtent.grow( 50. * 1000. );
        else if ( mSourceCRS.mapUnits() == QgsUnitTypes::DistanceDegrees )
          mComputedExtent.grow( 50. / 110 );
        QgsMessageLog::logMessage( tr( "Layer extent reported by the server is not correct. "
                                       "You may need to zoom on layer and then zoom out to see all features" ), tr( "WFS" ) );
      }
      mMutex.unlock();
      if ( !mComputedExtent.isNull() )
      {
        emit extentUpdated();
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
    QString msg( tr( "%1: The download limit has been reached." ).arg( mURI.typeName() ) );
    if ( !mRect.isEmpty() )
      msg += " " + tr( "Zoom in to fetch all data." );
    else
      msg += " " + tr( "You may want to check the 'Only request features overlapping the view extent' option to be able to zoom in to fetch all data." );
    QgsMessageLog::logMessage( msg, tr( "WFS" ) );
  }
}

// This is called by the destructor or QgsWFSProvider::reloadData(). The effect is to invalid
// all the caching state, so that a new request results in fresh download
void QgsWFSSharedData::invalidateCache()
{
  // Cf explanations in registerToCache() for the locking strategy
  QMutexLocker lockerMyself( &mMutexRegisterToCache );

  QMutexLocker locker( &mMutex );

// to prevent deadlock when waiting the end of the downloader thread that will try to take the mutex in serializeFeatures()
  mMutex.unlock();
  delete mDownloader;
  mMutex.lock();
  mDownloader = nullptr;
  mDownloadFinished = false;
  mGenCounter = 0;
  mCachedRegions = QgsSpatialIndex();
  mRegions.clear();
  mRect = QgsRectangle();
  mRequestLimit = 0;
  mGetFeatureHitsIssued = false;
  mFeatureCount = 0;
  mFeatureCountExact = false;
  mTotalFeaturesAttemptedToBeCached = 0;
  if ( !mCacheDbname.isEmpty() && mCacheDataProvider )
  {
    // We need to invalidate connections pointing to the cache, so as to
    // be able to delete the file.
    mCacheDataProvider->invalidateConnections( mCacheDbname );
  }
  delete mCacheDataProvider;
  mCacheDataProvider = nullptr;

  if ( !mCacheDbname.isEmpty() )
  {
    QFile::remove( mCacheDbname );
    QFile::remove( mCacheDbname + "-wal" );
    QFile::remove( mCacheDbname + "-shm" );
    mCacheDbname.clear();
  }
}

void QgsWFSSharedData::setFeatureCount( int featureCount )
{
  QMutexLocker locker( &mMutex );
  mGetFeatureHitsIssued = true;
  mFeatureCountExact = true;
  mFeatureCount = featureCount;
}

int QgsWFSSharedData::getFeatureCount( bool issueRequestIfNeeded )
{
  if ( !mGetFeatureHitsIssued && !mFeatureCountExact && mCaps.supportsHits && issueRequestIfNeeded )
  {
    mGetFeatureHitsIssued = true;
    QgsWFSFeatureHitsRequest request( mURI );
    int featureCount = request.getFeatureCount( mWFSVersion, mWFSFilter );

    {
      QMutexLocker locker( &mMutex );
      // Check the return value. Might be -1 in case of error, or might be
      // saturated by the server limit, but we may have retrieved more features
      // in the mean time.
      if ( featureCount > mFeatureCount )
      {
        // If the feature count is below or above than the server side limit, we know
        // that it is exact (some server implementation might saturate to the server side limit)
        if ( mCaps.maxFeatures > 0 && featureCount != mCaps.maxFeatures )
        {
          mFeatureCount = featureCount;
          mFeatureCountExact = true;
        }
        else if ( mCaps.maxFeatures <= 0 )
        {
          mFeatureCount = featureCount;
          mFeatureCountExact = true;
        }
      }
    }
  }
  return mFeatureCount;
}

QgsGmlStreamingParser *QgsWFSSharedData::createParser()
{
  QgsGmlStreamingParser::AxisOrientationLogic axisOrientationLogic( QgsGmlStreamingParser::Honour_EPSG_if_urn );
  if ( mURI.ignoreAxisOrientation() )
  {
    axisOrientationLogic = QgsGmlStreamingParser::Ignore_EPSG;
  }

  if ( !mLayerPropertiesList.isEmpty() )
  {
    QList< QgsGmlStreamingParser::LayerProperties > layerPropertiesList;
    Q_FOREACH ( QgsOgcUtils::LayerProperties layerProperties, mLayerPropertiesList )
    {
      QgsGmlStreamingParser::LayerProperties layerPropertiesOut;
      layerPropertiesOut.mName = layerProperties.mName;
      layerPropertiesOut.mGeometryAttribute = layerProperties.mGeometryAttribute;
      layerPropertiesList << layerPropertiesOut;
    }

    return new QgsGmlStreamingParser( layerPropertiesList,
                                      mFields,
                                      mMapFieldNameToSrcLayerNameFieldName,
                                      axisOrientationLogic,
                                      mURI.invertAxisOrientation() );
  }
  else
  {
    return new QgsGmlStreamingParser( mURI.typeName(),
                                      mGeometryAttribute,
                                      mFields,
                                      axisOrientationLogic,
                                      mURI.invertAxisOrientation() );
  }
}


// -------------------------


QgsWFSFeatureHitsRequest::QgsWFSFeatureHitsRequest( QgsWFSDataSourceURI &uri )
  : QgsWfsRequest( uri )
{
}

int QgsWFSFeatureHitsRequest::getFeatureCount( const QString &WFSVersion,
    const QString &filter )
{
  QUrl getFeatureUrl( mUri.requestUrl( QStringLiteral( "GetFeature" ) ) );
  getFeatureUrl.addQueryItem( QStringLiteral( "VERSION" ),  WFSVersion );
  if ( WFSVersion.startsWith( QLatin1String( "2.0" ) ) )
    getFeatureUrl.addQueryItem( QStringLiteral( "TYPENAMES" ), mUri.typeName() );
  else
    getFeatureUrl.addQueryItem( QStringLiteral( "TYPENAME" ), mUri.typeName() );
  if ( !filter.isEmpty() )
  {
    getFeatureUrl.addQueryItem( QStringLiteral( "FILTER" ), filter );
  }
  getFeatureUrl.addQueryItem( QStringLiteral( "RESULTTYPE" ), QStringLiteral( "hits" ) );

  if ( !sendGET( getFeatureUrl, true ) )
    return -1;

  const QByteArray &buffer = response();

  QgsDebugMsgLevel( QStringLiteral( "parsing QgsWFSFeatureHitsRequest: " ) + buffer, 4 );

  // parse XML
  QString error;
  QDomDocument domDoc;
  if ( !domDoc.setContent( buffer, true, &error ) )
  {
    QgsDebugMsg( QStringLiteral( "parsing failed: " ) + error );
    return -1;
  }

  QDomElement doc = domDoc.documentElement();
  QString numberOfFeatures =
    ( WFSVersion.startsWith( QLatin1String( "1.1" ) ) ) ? doc.attribute( QStringLiteral( "numberOfFeatures" ) ) :
    /* 2.0 */                         doc.attribute( QStringLiteral( "numberMatched" ) );
  if ( !numberOfFeatures.isEmpty() )
  {
    bool isValid;
    int ret = numberOfFeatures.toInt( &isValid );
    if ( !isValid )
      return -1;
    return ret;
  }

  return -1;
}

QString QgsWFSFeatureHitsRequest::errorMessageWithReason( const QString &reason )
{
  return tr( "Download of feature count failed: %1" ).arg( reason );
}


// -------------------------


QgsWFSSingleFeatureRequest::QgsWFSSingleFeatureRequest( QgsWFSSharedData *shared )
  : QgsWfsRequest( shared->mURI ), mShared( shared )
{
}

QgsRectangle QgsWFSSingleFeatureRequest::getExtent()
{
  QUrl getFeatureUrl( mUri.requestUrl( QStringLiteral( "GetFeature" ) ) );
  getFeatureUrl.addQueryItem( QStringLiteral( "VERSION" ),  mShared->mWFSVersion );
  if ( mShared->mWFSVersion .startsWith( QLatin1String( "2.0" ) ) )
    getFeatureUrl.addQueryItem( QStringLiteral( "TYPENAMES" ), mUri.typeName() );
  else
    getFeatureUrl.addQueryItem( QStringLiteral( "TYPENAME" ), mUri.typeName() );
  if ( mShared->mWFSVersion .startsWith( QLatin1String( "2.0" ) ) )
    getFeatureUrl.addQueryItem( QStringLiteral( "COUNT" ), QString::number( 1 ) );
  else
    getFeatureUrl.addQueryItem( QStringLiteral( "MAXFEATURES" ), QString::number( 1 ) );

  if ( !sendGET( getFeatureUrl, true ) )
    return QgsRectangle();

  const QByteArray &buffer = response();

  QgsDebugMsgLevel( QStringLiteral( "parsing QgsWFSSingleFeatureRequest: " ) + buffer, 4 );

  // parse XML
  QgsGmlStreamingParser *parser = mShared->createParser();
  QString gmlProcessErrorMsg;
  QgsRectangle extent;
  if ( parser->processData( buffer, true, gmlProcessErrorMsg ) )
  {
    QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> featurePtrList =
      parser->getAndStealReadyFeatures();
    for ( int i = 0; i < featurePtrList.size(); i++ )
    {
      QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair &featPair = featurePtrList[i];
      QgsFeature f( *( featPair.first ) );
      QgsGeometry geometry = f.geometry();
      if ( !geometry.isNull() )
      {
        extent = geometry.boundingBox();
      }
      delete featPair.first;
    }
  }
  delete parser;
  return extent;
}

QString QgsWFSSingleFeatureRequest::errorMessageWithReason( const QString &reason )
{
  return tr( "Download of feature failed: %1" ).arg( reason );
}


