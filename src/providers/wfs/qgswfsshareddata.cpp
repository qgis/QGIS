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

#include <math.h> // M_PI

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
#include "qgsslconnect.h"

#include <cpl_vsi.h>
#include <cpl_conv.h>
#include <ogr_api.h>

#include <sqlite3.h>

QgsWFSSharedData::QgsWFSSharedData( const QString& uri )
    : mURI( uri )
    , mSourceCRS( 0 )
    , mCacheDataProvider( nullptr )
    , mMaxFeatures( 0 )
    , mMaxFeaturesWasSetFromDefaultForPaging( false )
    , mHideProgressDialog( mURI.hideDownloadProgressDialog() )
    , mDistinctSelect( false )
    , mHasWarnedAboutMissingFeatureId( false )
    , mGetFeatureEPSGDotHonoursEPSGOrder( false )
    , mDownloader( nullptr )
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
  QgsDebugMsg( QString( "~QgsWFSSharedData()" ) );

  invalidateCache();
}

QString QgsWFSSharedData::srsName() const
{
  QString srsName;
  if ( !mSourceCRS.authid().isEmpty() )
  {
    if ( mWFSVersion.startsWith( "1.0" ) ||
         !mSourceCRS.authid().startsWith( "EPSG:" ) ||
         // For servers like Geomedia that advertize EPSG:XXXX in capabilities even in WFS 1.1 or 2.0
         mCaps.useEPSGColumnFormat )
    {
      srsName = mSourceCRS.authid();
    }
    else
    {
      QStringList list = mSourceCRS.authid().split( ':' );
      srsName = QString( "urn:ogc:def:crs:EPSG::%1" ).arg( list.last() );
    }
  }
  return srsName;
}

bool QgsWFSSharedData::computeFilter( QString& errorMsg )
{
  errorMsg.clear();
  mWFSFilter.clear();
  mSortBy.clear();

  QgsOgcUtils::GMLVersion gmlVersion;
  QgsOgcUtils::FilterVersion filterVersion;
  bool honourAxisOrientation = false;
  if ( mWFSVersion.startsWith( "1.0" ) )
  {
    gmlVersion = QgsOgcUtils::GML_2_1_2;
    filterVersion = QgsOgcUtils::FILTER_OGC_1_0;
  }
  else if ( mWFSVersion.startsWith( "1.1" ) )
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

    const QgsSQLStatement::NodeSelect* select = dynamic_cast<const QgsSQLStatement::NodeSelect*>( sql.rootNode() );
    if ( !select )
    {
      // Makes Coverity happy, but cannot happen in practice
      QgsDebugMsg( "should not happen" );
      return false;
    }
    QList<QgsSQLStatement::NodeColumnSorted*> orderBy = select->orderBy();
    Q_FOREACH ( QgsSQLStatement::NodeColumnSorted* columnSorted, orderBy )
    {
      if ( !mSortBy.isEmpty() )
        mSortBy += ",";
      mSortBy += columnSorted->column()->name();
      if ( !columnSorted->ascending() )
      {
        if ( mWFSVersion.startsWith( "2.0" ) )
          mSortBy += " DESC";
        else
          mSortBy += " D";
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
// incompatible with how the QGIS spatialite provider works.
// The symptom of the issue is the error message
// 'Unable to Initialize SpatiaLite Metadata: no such function: InitSpatialMetadata'
// So in that case we must use only QGIS functions to avoid the conflict
// The difference is that in the QGIS way we have to create the template database
// on disk, which is a slightly bit slower. But due to later caching, this is
// not so a big deal.
#if GDAL_VERSION_MAJOR >= 2 || GDAL_VERSION_MINOR >= 11
#define USE_OGR_FOR_DB_CREATION
#endif

static QString quotedIdentifier( QString id )
{
  id.replace( '\"', "\"\"" );
  return id.prepend( '\"' ).append( '\"' );
}

bool QgsWFSSharedData::createCache()
{
  Q_ASSERT( mCacheDbname.isEmpty() );

  static int tmpCounter = 0;
  ++tmpCounter;
  mCacheDbname =  QDir( QgsWFSUtils::acquireCacheDirectory() ).filePath( QString( "wfs_cache_%1.sqlite" ).arg( tmpCounter ) );

  QgsFields cacheFields;
  Q_FOREACH ( const QgsField& field, mFields )
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
  cacheFields.append( QgsField( QgsWFSConstants::FIELD_GEN_COUNTER, QVariant::Int, "int" ) );
  cacheFields.append( QgsField( QgsWFSConstants::FIELD_GMLID, QVariant::String, "string" ) );
  cacheFields.append( QgsField( QgsWFSConstants::FIELD_HEXWKB_GEOM, QVariant::String, "string" ) );
  if ( mDistinctSelect )
    cacheFields.append( QgsField( QgsWFSConstants::FIELD_MD5, QVariant::String, "string" ) );

  bool ogrWaySuccessful = false;
  QString fidName( "__ogc_fid" );
  QString geometryFieldname( "__spatialite_geometry" );
#ifdef USE_OGR_FOR_DB_CREATION
  // Only GDAL >= 2.0 can use an alternate geometry or FID field name
  // but QgsVectorFileWriter will refuse anyway to create a ogc_fid, so we will
  // do it manually
  bool useReservedNames = cacheFields.fieldNameIndex( "ogc_fid" ) >= 0;
#if GDAL_VERSION_MAJOR < 2
  if ( cacheFields.fieldNameIndex( "geometry" ) >= 0 )
    useReservedNames = true;
#endif
  if ( !useReservedNames )
  {
#if GDAL_VERSION_MAJOR < 2
    fidName = "ogc_fid";
    geometryFieldname = "GEOMETRY";
#endif
    // Creating a spatialite database can be quite slow on some file systems
    // so we create a GDAL in-memory file, and then copy it on
    // the file system.
    QString vsimemFilename;
    QStringList datasourceOptions;
    QStringList layerOptions;
    datasourceOptions.push_back( "INIT_WITH_EPSG=NO" );
    layerOptions.push_back( "LAUNDER=NO" ); // to get exact matches for field names, especially regarding case
#if GDAL_VERSION_MAJOR >= 2
    layerOptions.push_back( "FID=__ogc_fid" );
    layerOptions.push_back( "GEOMETRY_NAME=__spatialite_geometry" );
#endif
    vsimemFilename.sprintf( "/vsimem/qgis_wfs_cache_template_%p/features.sqlite", this );
    mCacheTablename = CPLGetBasename( vsimemFilename.toStdString().c_str() );
    VSIUnlink( vsimemFilename.toStdString().c_str() );
    QgsVectorFileWriter* writer = new QgsVectorFileWriter( vsimemFilename, "",
        cacheFields, QGis::WKBPolygon, nullptr, "SpatiaLite", datasourceOptions, layerOptions );
    if ( writer->hasError() == QgsVectorFileWriter::NoError )
    {
      delete writer;

      // Copy the temporary database back to disk
      vsi_l_offset nLength = 0;
      GByte* pabyData = VSIGetMemFileBuffer( vsimemFilename.toStdString().c_str(), &nLength, TRUE );
      VSILFILE* fp = VSIFOpenL( mCacheDbname.toStdString().c_str(), "wb " );
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
      delete writer;
      VSIUnlink( vsimemFilename.toStdString().c_str() );
    }
  }
#endif
  if ( !ogrWaySuccessful )
  {
    static QMutex mutexDBnameCreation;
    static QByteArray cachedDBTemplate;
    QMutexLocker mutexDBnameCreationHolder( &mutexDBnameCreation );
    if ( cachedDBTemplate.size() == 0 )
    {
      // Create a template Spatialite DB
      QTemporaryFile tempFile;
      tempFile.open();
      tempFile.setAutoRemove( false );
      tempFile.close();
      QString spatialite_lib = QgsProviderRegistry::instance()->library( "spatialite" );
      QLibrary* myLib = new QLibrary( spatialite_lib );
      bool loaded = myLib->load();
      bool created = false;
      if ( loaded )
      {
        QgsDebugMsg( "spatialite provider loaded" );

        typedef bool ( *createDbProc )( const QString&, QString& );
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
        cachedDBTemplate = file.readAll();
      file.close();
      QFile::remove( tempFile.fileName() );
    }

    // Copy the in-memory template Spatialite DB into the target DB
    QFile dbFile( mCacheDbname );
    if ( !dbFile.open( QIODevice::WriteOnly ) )
    {
      QgsMessageLog::logMessage( tr( "Cannot create temporary SpatiaLite cache" ), tr( "WFS" ) );
      return false;
    }
    if ( dbFile.write( cachedDBTemplate ) < 0 )
    {
      QgsMessageLog::logMessage( tr( "Cannot create temporary SpatiaLite cache" ), tr( "WFS" ) );
      return false;
    }
  }

  sqlite3 *db;
  bool ret = true;
  int rc = QgsSLConnect::sqlite3_open( mCacheDbname.toUtf8(), &db );
  if ( rc == SQLITE_OK )
  {
    QString sql;

    ( void )sqlite3_exec( db, "PRAGMA synchronous=OFF", nullptr, nullptr, nullptr );
    // WAL is needed to avoid reader to block writers
    ( void )sqlite3_exec( db, "PRAGMA journal_mode=WAL", nullptr, nullptr, nullptr );

    ( void )sqlite3_exec( db, "BEGIN", nullptr, nullptr, nullptr );

    if ( !ogrWaySuccessful )
    {
      mCacheTablename = "features";
      sql = QString( "CREATE TABLE %1 (%2 INTEGER PRIMARY KEY" ).arg( mCacheTablename ).arg( fidName );
      Q_FOREACH ( QgsField field, cacheFields )
      {
        QString type( "VARCHAR" );
        if ( field.type() == QVariant::Int )
          type = "INTEGER";
        else if ( field.type() == QVariant::LongLong )
          type = "BIGINT";
        else if ( field.type() == QVariant::Double )
          type = "REAL";
        sql += QString( ", %1 %2" ).arg( quotedIdentifier( field.name() ), type );
      }
      sql += ")";
      rc = sqlite3_exec( db, sql.toUtf8(), nullptr, nullptr, nullptr );
      if ( rc != SQLITE_OK )
      {
        QgsDebugMsg( QString( "%1 failed" ).arg( sql ) );
        ret = false;
      }

      sql = QString( "SELECT AddGeometryColumn('%1','%2',0,'POLYGON',2)" ).arg( mCacheTablename, geometryFieldname );
      rc = sqlite3_exec( db, sql.toUtf8(), nullptr, nullptr, nullptr );
      if ( rc != SQLITE_OK )
      {
        QgsDebugMsg( QString( "%1 failed" ).arg( sql ) );
        ret = false;
      }

      sql = QString( "SELECT CreateSpatialIndex('%1','%2')" ).arg( mCacheTablename, geometryFieldname );
      rc = sqlite3_exec( db, sql.toUtf8(), nullptr, nullptr, nullptr );
      if ( rc != SQLITE_OK )
      {
        QgsDebugMsg( QString( "%1 failed" ).arg( sql ) );
        ret = false;
      }
    }

    // We need an index on the gmlid, since we will check for duplicates, particularly
    // useful in the case we do overlapping BBOX requests
    sql = QString( "CREATE INDEX idx_%2 ON %1(%2)" ).arg( mCacheTablename ).arg( QgsWFSConstants::FIELD_GMLID );
    rc = sqlite3_exec( db, sql.toUtf8(), nullptr, nullptr, nullptr );
    if ( rc != SQLITE_OK )
    {
      QgsDebugMsg( QString( "%1 failed" ).arg( sql ) );
      ret = false;
    }

    if ( mDistinctSelect )
    {
      sql = QString( "CREATE INDEX idx_%2 ON %1(%2)" ).arg( mCacheTablename ).arg( QgsWFSConstants::FIELD_MD5 );
      rc = sqlite3_exec( db, sql.toUtf8(), nullptr, nullptr, nullptr );
      if ( rc != SQLITE_OK )
      {
        QgsDebugMsg( QString( "%1 failed" ).arg( sql ) );
        ret = false;
      }
    }

    ( void )sqlite3_exec( db, "COMMIT", nullptr, nullptr, nullptr );

    QgsSLConnect::sqlite3_close( db );
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
  QgsDataSourceURI dsURI;
  dsURI.setDatabase( mCacheDbname );
  dsURI.setDataSource( "", mCacheTablename, geometryFieldname, "", fidName );
  QStringList pragmas;
  pragmas << "synchronous=OFF";
  pragmas << "journal_mode=WAL"; // WAL is needed to avoid reader to block writers
  dsURI.setParam( "pragma", pragmas );
  mCacheDataProvider = ( QgsVectorDataProvider* )( QgsProviderRegistry::instance()->provider(
                         "spatialite", dsURI.uri() ) );
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

  return true;
}

int QgsWFSSharedData::registerToCache( QgsWFSFeatureIterator* iterator, QgsRectangle rect )
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
      if ( mRegions[id].geometry()->boundingBox().contains( rect ) &&
           !mRegions[id].attributes().value( 0 ).toBool() )
      {
        QgsDebugMsg( "Cached features already cover this area of interest" );
        newDownloadNeeded = false;
        break;
      }

      // On the other hand, if the requested bbox is inside an already cached rect,
      // that hit the download limit, our larger bbox will hit it too, so no need
      // to re-issue a new request either.
      if ( rect.contains( mRegions[id].geometry()->boundingBox() ) &&
           mRegions[id].attributes().value( 0 ).toBool() )
      {
        QgsDebugMsg( "Current request is larger than a smaller request that hit the download limit, so no server download needed." );
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

  if ( newDownloadNeeded || !mDownloader )
  {
    mRect = rect;
    // to prevent deadlock when waiting the end of the downloader thread that will try to take the mutex in serializeFeatures()
    mMutex.unlock();
    delete mDownloader;
    mMutex.lock();
    mDownloadFinished = false;
    mComputedExtent = QgsRectangle();
    mDownloader = new QgsWFSThreadedFeatureDownloader( this );
    QEventLoop loop;
    connect( mDownloader, SIGNAL( ready() ), &loop, SLOT( quit() ) );
    mDownloader->start();
    loop.exec( QEventLoop::ExcludeUserInputEvents );
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

QSet<QString> QgsWFSSharedData::getExistingCachedGmlIds( const QVector<QgsWFSFeatureGmlIdPair>& featureList )
{
  QString expr;
  bool first = true;
  QSet<QString> setExistingGmlIds;

  const QgsFields & dataProviderFields = mCacheDataProvider->fields();
  const int gmlidIdx = dataProviderFields.indexFromName( QgsWFSConstants::FIELD_GMLID );

  // To avoid excessive memory consumption in expression building, do not
  // query more than 1000 ids at a time.
  for ( int i = 0; i < featureList.size(); i ++ )
  {
    if ( !first )
      expr += ",";
    else
    {
      expr = QgsWFSConstants::FIELD_GMLID + " IN (";
      first = false;
    }
    expr += "'";
    expr += featureList[i].second;
    expr += "'";

    if (( i > 0 && ( i % 1000 ) == 0 ) || i + 1 == featureList.size() )
    {
      expr += ")";

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

QSet<QString> QgsWFSSharedData::getExistingCachedMD5( const QVector<QgsWFSFeatureGmlIdPair>& featureList )
{
  QString expr;
  bool first = true;
  QSet<QString> setExistingMD5;

  const QgsFields & dataProviderFields = mCacheDataProvider->fields();
  const int md5Idx = dataProviderFields.indexFromName( QgsWFSConstants::FIELD_MD5 );

  // To avoid excessive memory consumption in expression building, do not
  // query more than 1000 ids at a time.
  for ( int i = 0; i < featureList.size(); i ++ )
  {
    if ( !first )
      expr += ",";
    else
    {
      expr = QgsWFSConstants::FIELD_MD5 + " IN (";
      first = false;
    }
    expr += "'";
    expr += QgsWFSUtils::getMD5( featureList[i].first );
    expr += "'";

    if (( i > 0 && ( i % 1000 ) == 0 ) || i + 1 == featureList.size() )
    {
      expr += ")";

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
  if ( !mCacheDataProvider )
    return QString();
  QgsFeatureRequest request;
  request.setFilterFid( fid );

  const QgsFields & dataProviderFields = mCacheDataProvider->fields();
  int gmlidIdx = dataProviderFields.indexFromName( QgsWFSConstants::FIELD_GMLID );

  QgsAttributeList attList;
  attList.append( gmlidIdx );
  request.setSubsetOfAttributes( attList );

  QgsFeatureIterator iterGmlIds( mCacheDataProvider->getFeatures( request ) );
  QgsFeature gmlidFeature;
  QSet<QString> setExistingGmlIds;
  while ( iterGmlIds.nextFeature( gmlidFeature ) )
  {
    const QVariant &v = gmlidFeature.attributes().value( gmlidIdx );
    return v.toString();
  }
  return QString();
}

// Used by WFS-T
bool QgsWFSSharedData::deleteFeatures( const QgsFeatureIds& fidlist )
{
  if ( !mCacheDataProvider )
    return false;

  {
    QMutexLocker locker( &mMutex );
    mFeatureCount -= fidlist.size();
  }

  return mCacheDataProvider->deleteFeatures( fidlist );
}

// Used by WFS-T
bool QgsWFSSharedData::changeGeometryValues( const QgsGeometryMap &geometry_map )
{
  if ( !mCacheDataProvider )
    return false;

  // We need to replace the geometry by its bounding box and issue a attribute
  // values change with the real geometry serialized as hexwkb.

  int idx = mCacheDataProvider->fields().indexFromName( QgsWFSConstants::FIELD_HEXWKB_GEOM );
  Q_ASSERT( idx >= 0 );

  QgsGeometryMap newGeometryMap;
  QgsChangedAttributesMap newChangedAttrMap;
  for ( QgsGeometryMap::const_iterator iter = geometry_map.constBegin(); iter != geometry_map.constEnd(); ++iter )
  {
    const unsigned char *geom = iter->asWkb();
    int geomSize = iter->wkbSize();
    if ( geomSize )
    {
      QgsAttributeMap newAttrMap;
      QByteArray array(( const char* )geom, geomSize );
      newAttrMap[idx] = QString( array.toHex().data() );
      newChangedAttrMap[ iter.key()] = newAttrMap;

      QgsGeometry* polyBoudingBox = QgsGeometry::fromRect( iter.value().boundingBox() );
      newGeometryMap[ iter.key()] = *polyBoudingBox;
      delete polyBoudingBox;
    }
    else
    {
      QgsAttributeMap newAttrMap;
      newAttrMap[idx] = QString();
      newChangedAttrMap[ iter.key()] = newAttrMap;
      newGeometryMap[ iter.key()] = QgsGeometry();
    }
  }

  return mCacheDataProvider->changeGeometryValues( newGeometryMap ) &&
         mCacheDataProvider->changeAttributeValues( newChangedAttrMap );
}

// Used by WFS-T
bool QgsWFSSharedData::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  if ( !mCacheDataProvider )
    return false;

  const QgsFields & dataProviderFields = mCacheDataProvider->fields();
  QgsChangedAttributesMap newMap;
  for ( QgsChangedAttributesMap::const_iterator iter = attr_map.begin(); iter != attr_map.end(); ++iter )
  {
    QgsFeatureId fid = iter.key();
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
    newMap[fid] = newAttrMap;
  }

  return mCacheDataProvider->changeAttributeValues( newMap );
}

void QgsWFSSharedData::serializeFeatures( QVector<QgsWFSFeatureGmlIdPair>& featureList )
{
  QgsDebugMsg( QString( "begin %1" ).arg( featureList.size() ) );

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
  const QgsFields & dataProviderFields = mCacheDataProvider->fields();
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
  Q_FOREACH ( QgsWFSFeatureGmlIdPair featPair, featureList )
  {
    const QgsFeature& gmlFeature = featPair.first;

    const QString& gmlId = featPair.second;
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
          QgsDebugMsg( QString( "duplicate gmlId %1" ).arg( gmlId ) );
        }
        continue;
      }
      else
      {
        existingGmlIds.insert( gmlId );
      }
    }

    updatedFeatureList.push_back( featPair );

    QgsFeature cachedFeature;
    cachedFeature.initAttributes( dataProviderFields.size() );

    //copy the geometry
    const QgsGeometry* geometry = gmlFeature.constGeometry();
    if ( !mGeometryAttribute.isEmpty() && geometry )
    {
      const unsigned char *geom = geometry->asWkb();
      int geomSize = geometry->wkbSize();
      QByteArray array(( const char* )geom, geomSize );

      cachedFeature.setAttribute( hexwkbGeomIdx, QVariant( QString( array.toHex().data() ) ) );

      QgsRectangle bBox( geometry->boundingBox() );
      if ( localComputedExtent.isNull() )
        localComputedExtent = bBox;
      else
        localComputedExtent.combineExtentWith( bBox );
      QgsGeometry* polyBoundingBox = QgsGeometry::fromRect( bBox );
      cachedFeature.setGeometry( polyBoundingBox );
    }
    else
    {
      cachedFeature.setAttribute( hexwkbGeomIdx, QVariant( QString() ) );
    }

    //and the attributes
    for ( int i = 0; i < mFields.size(); i++ )
    {
      int idx = dataProviderFields.indexFromName( mFields.at( i ).name() );
      if ( idx >= 0 )
      {
        const QVariant &v = gmlFeature.attributes().value( i );
        if ( v.type() == QVariant::DateTime && !v.isNull() )
          cachedFeature.setAttribute( idx, QVariant( v.toDateTime().toMSecsSinceEpoch() ) );
        else if ( v.type() !=  dataProviderFields.at( idx ).type() )
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
    // That way we will always have a consistant feature id, even in case of
    // paging or BBOX request
    Q_ASSERT( featureListToCache.size() == updatedFeatureList.size() );
    for ( int i = 0; i < updatedFeatureList.size(); i++ )
    {
      if ( cacheOk )
        updatedFeatureList[i].first.setFeatureId( featureListToCache[i].id() );
      else
        updatedFeatureList[i].first.setFeatureId( mTotalFeaturesAttemptedToBeCached + i + 1 );
    }

    {
      QMutexLocker locker( &mMutex );
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

  featureList = updatedFeatureList;

  emit extentUpdated();

  QgsDebugMsg( QString( "end %1" ).arg( featureList.size() ) );

}

QgsRectangle QgsWFSSharedData::computedExtent()
{
  QMutexLocker locker( &mMutex );
  return mComputedExtent;
}

void QgsWFSSharedData::pushError( const QString& errorMsg )
{
  QgsMessageLog::logMessage( errorMsg, tr( "WFS" ) );
  emit raiseError( errorMsg );
}

/** Called by QgsWFSFeatureDownloader::run() at the end of the download process. */
void QgsWFSSharedData::endOfDownload( bool success, int featureCount,
                                      bool truncatedResponse,
                                      bool interrupted,
                                      QString errorMsg )
{
  QMutexLocker locker( &mMutex );

  if ( !success && !interrupted )
  {
    QString errorMsgOut( tr( "Download of features for layer %1 failed or partially failed: %2. You may attempt reloading the layer with F5" ).arg( mURI.typeName() ).arg( errorMsg ) );
    pushError( errorMsgOut );
  }

  bool bDownloadLimit = truncatedResponse || ( !mCaps.supportsPaging && featureCount == mMaxFeatures && mMaxFeatures > 0 );

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
      QgsDebugMsg( "Capability extent is probably wrong. Starting a new request with one feature limit to get at least one feature" );
      mTryFetchingOneFeature = true;
      QgsWFSSingleFeatureRequest request( this );
      mComputedExtent = request.getExtent();
      if ( !mComputedExtent.isNull() )
      {
        // Grow the extent by ~ 50 km (completely arbitrary number if you wonder!)
        // so that it is sufficiently zoomed out
        if ( mSourceCRS.mapUnits() == QGis::Meters )
          mComputedExtent.grow( 50. * 1000. );
        else if ( mSourceCRS.mapUnits() == QGis::Degrees )
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

    // In case the download was successful, we will remember this bbox
    // and if the download reached the download limit or not
    QgsFeature f;
    f.setGeometry( QgsGeometry::fromRect( mRect ) );
    QgsFeatureId id = mRegions.size();
    f.setFeatureId( id );
    f.initAttributes( 1 );
    f.setAttribute( 0, QVariant( bDownloadLimit ) );
    mRegions.push_back( f );
    mCachedRegions.insertFeature( f );
  }

  if ( mRect.isEmpty() && success && !bDownloadLimit && !mFeatureCountExact )
  {
    mFeatureCountExact = true;
    if ( featureCount != mFeatureCount )
    {
      // Shouldn't happen unless there's a bug somewhere or the result returned
      // contains duplicates. Actually happens with
      // http://demo.opengeo.org/geoserver/wfs?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=osm:landcover_line
      // with gml.id=landcover_line.119246130 in particular
      QgsDebugMsg( QString( "raw features=%1, unique features=%2" ).
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
    QgsMessageLog::logMessage( msg, "WFS" );
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
    QgsWFSUtils::releaseCacheDirectory();
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

QgsGmlStreamingParser* QgsWFSSharedData::createParser()
{
  QgsGmlStreamingParser::AxisOrientationLogic axisOrientationLogic( QgsGmlStreamingParser::Honour_EPSG_if_urn );
  if ( mURI.ignoreAxisOrientation() )
  {
    axisOrientationLogic = QgsGmlStreamingParser::Ignore_EPSG;
  }

  if ( mLayerPropertiesList.size() )
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


QgsWFSFeatureHitsRequest::QgsWFSFeatureHitsRequest( QgsWFSDataSourceURI& uri )
    : QgsWFSRequest( uri.uri() )
{
}

QgsWFSFeatureHitsRequest::~QgsWFSFeatureHitsRequest()
{
}

int QgsWFSFeatureHitsRequest::getFeatureCount( const QString& WFSVersion,
    const QString& filter )
{
  QUrl getFeatureUrl( mUri.baseURL() );
  getFeatureUrl.addQueryItem( "REQUEST", "GetFeature" );
  getFeatureUrl.addQueryItem( "VERSION",  WFSVersion );
  if ( WFSVersion.startsWith( "2.0" ) )
    getFeatureUrl.addQueryItem( "TYPENAMES", mUri.typeName() );
  else
    getFeatureUrl.addQueryItem( "TYPENAME", mUri.typeName() );
  if ( !filter.isEmpty() )
  {
    getFeatureUrl.addQueryItem( "FILTER", filter );
  }
  getFeatureUrl.addQueryItem( "RESULTTYPE", "hits" );

  if ( !sendGET( getFeatureUrl, true ) )
    return -1;

  const QByteArray& buffer = response();

  QgsDebugMsg( "parsing QgsWFSFeatureHitsRequest: " + buffer );

  // parse XML
  QString error;
  QDomDocument domDoc;
  if ( !domDoc.setContent( buffer, true, &error ) )
  {
    QgsDebugMsg( "parsing failed: " + error );
    return -1;
  }

  QDomElement doc = domDoc.documentElement();
  QString numberOfFeatures =
    ( WFSVersion.startsWith( "1.1" ) ) ? doc.attribute( "numberOfFeatures" ) :
    /* 2.0 */                         doc.attribute( "numberMatched" ) ;
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

QString QgsWFSFeatureHitsRequest::errorMessageWithReason( const QString& reason )
{
  return tr( "Download of feature count failed: %1" ).arg( reason );
}


// -------------------------


QgsWFSSingleFeatureRequest::QgsWFSSingleFeatureRequest( QgsWFSSharedData* shared )
    : QgsWFSRequest( shared->mURI.uri() ), mShared( shared )
{
}

QgsWFSSingleFeatureRequest::~QgsWFSSingleFeatureRequest()
{
}

QgsRectangle QgsWFSSingleFeatureRequest::getExtent()
{
  QUrl getFeatureUrl( mUri.baseURL() );
  getFeatureUrl.addQueryItem( "REQUEST", "GetFeature" );
  getFeatureUrl.addQueryItem( "VERSION",  mShared->mWFSVersion );
  if ( mShared->mWFSVersion .startsWith( "2.0" ) )
    getFeatureUrl.addQueryItem( "TYPENAMES", mUri.typeName() );
  else
    getFeatureUrl.addQueryItem( "TYPENAME", mUri.typeName() );
  if ( mShared->mWFSVersion .startsWith( "2.0" ) )
    getFeatureUrl.addQueryItem( "COUNT", QString::number( 1 ) );
  else
    getFeatureUrl.addQueryItem( "MAXFEATURES", QString::number( 1 ) );

  if ( !sendGET( getFeatureUrl, true ) )
    return -1;

  const QByteArray& buffer = response();

  QgsDebugMsg( "parsing QgsWFSSingleFeatureRequest: " + buffer );

  // parse XML
  QgsGmlStreamingParser* parser = mShared->createParser();
  QString gmlProcessErrorMsg;
  QgsRectangle extent;
  if ( parser->processData( buffer, true, gmlProcessErrorMsg ) )
  {
    QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> featurePtrList =
      parser->getAndStealReadyFeatures();
    QVector<QgsWFSFeatureGmlIdPair> featureList;
    for ( int i = 0;i < featurePtrList.size();i++ )
    {
      QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair& featPair = featurePtrList[i];
      QgsFeature f( *( featPair.first ) );
      const QgsGeometry* geometry = f.constGeometry();
      if ( geometry )
      {
        extent = geometry->boundingBox();
      }
      delete featPair.first;
    }
  }
  delete parser;
  return extent;
}

QString QgsWFSSingleFeatureRequest::errorMessageWithReason( const QString& reason )
{
  return tr( "Download of feature failed: %1" ).arg( reason );
}


