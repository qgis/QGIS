/***************************************************************************
  qgsgdalprovider.cpp  -  QGIS Data provider for
                           GDAL rasters
                             -------------------
    begin                : November, 2010
    copyright            : (C) 2010 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslogger.h"
#include "qgsgdalproviderbase.h"
#include "qgsgdalprovider.h"
#include "qgsconfig.h"

#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgscoordinatetransform.h"
#include "qgsdataitem.h"
#include "qgsdataitemprovider.h"
#include "qgsdatasourceuri.h"
#include "qgsgdaldataitems.h"
#include "qgshtmlutils.h"
#include "qgsmessagelog.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsrasterbandstats.h"
#include "qgsrasteridentifyresult.h"
#include "qgsrasterlayer.h"
#include "qgsrasterpyramid.h"
#include "qgspointxy.h"
#include "qgssettings.h"
#include "qgsogrutils.h"

#ifdef HAVE_GUI
#include "qgssourceselectprovider.h"
#include "qgsgdalsourceselect.h"
#endif

#include <QImage>
#include <QColor>
#include <QProcess>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QHash>
#include <QTime>
#include <QTextDocument>
#include <QDebug>

#include <gdalwarper.h>
#include <gdal.h>
#include <ogr_srs_api.h>
#include <cpl_conv.h>
#include <cpl_string.h>

#define ERRMSG(message) QGS_ERROR_MESSAGE(message,"GDAL provider")
#define ERR(message) QgsError(message,"GDAL provider")

static QString PROVIDER_KEY = QStringLiteral( "gdal" );
static QString PROVIDER_DESCRIPTION = QStringLiteral( "GDAL provider" );

// To avoid potential races when destroying related instances ("main" and clones)
static QMutex gGdaProviderMutex( QMutex::Recursive );

QHash< QgsGdalProvider *, QVector<QgsGdalProvider::DatasetPair> > QgsGdalProvider::mgDatasetCache;

int QgsGdalProvider::mgDatasetCacheSize = 0;

// Number of cached datasets from which we will try to do eviction when a
// provider has 2 or more cached datasets
const int MIN_THRESHOLD_FOR_CACHE_CLEANUP = 10;

// Maximum number of cached datasets
// We try to keep at least 1 cached dataset per parent provider between
// MIN_THRESHOLD_FOR_CACHE_CLEANUP and MAX_CACHE_SIZE. But we don't want to
// maintain more than MAX_CACHE_SIZE datasets opened to avoid running short of
// file descriptors.
const int MAX_CACHE_SIZE = 50;

struct QgsGdalProgress
{
  int type;
  QgsGdalProvider *provider = nullptr;
  QgsRasterBlockFeedback *feedback = nullptr;
};
//
// global callback function
//
int CPL_STDCALL progressCallback( double dfComplete,
                                  const char *pszMessage,
                                  void *pProgressArg )
{
  Q_UNUSED( pszMessage );

  static double sDfLastComplete = -1.0;

  QgsGdalProgress *prog = static_cast<QgsGdalProgress *>( pProgressArg );

  if ( sDfLastComplete > dfComplete )
  {
    if ( sDfLastComplete >= 1.0 )
      sDfLastComplete = -1.0;
    else
      sDfLastComplete = dfComplete;
  }

  if ( std::floor( sDfLastComplete * 10 ) != std::floor( dfComplete * 10 ) )
  {
    if ( prog->feedback )
      prog->feedback->setProgress( dfComplete * 100 );
  }
  sDfLastComplete = dfComplete;

  if ( prog->feedback && prog->feedback->isCanceled() )
    return false;

  return true;
}

QgsGdalProvider::QgsGdalProvider( const QString &uri, const QgsError &error )
  : QgsRasterDataProvider( uri, QgsDataProvider::ProviderOptions() )
  , mpRefCounter( new QAtomicInt( 1 ) )
  , mpLightRefCounter( new QAtomicInt( 1 ) )
  , mUpdate( false )
{
  mGeoTransform[0] = 0;
  mGeoTransform[1] = 1;
  mGeoTransform[2] = 0;
  mGeoTransform[3] = 0;
  mGeoTransform[4] = 0;
  mGeoTransform[5] = -1;
  setError( error );
}

QgsGdalProvider::QgsGdalProvider( const QString &uri, const ProviderOptions &options, bool update, GDALDatasetH dataset )
  : QgsRasterDataProvider( uri, options )
  , mpRefCounter( new QAtomicInt( 1 ) )
  , mpMutex( new QMutex( QMutex::Recursive ) )
  , mpParent( new QgsGdalProvider * ( this ) )
  , mpLightRefCounter( new QAtomicInt( 1 ) )
  , mUpdate( update )
{
  mGeoTransform[0] = 0;
  mGeoTransform[1] = 1;
  mGeoTransform[2] = 0;
  mGeoTransform[3] = 0;
  mGeoTransform[4] = 0;
  mGeoTransform[5] = -1;

  QgsDebugMsg( "constructing with uri '" + uri + "'." );

  QgsGdalProviderBase::registerGdalDrivers();

  if ( !CPLGetConfigOption( "AAIGRID_DATATYPE", nullptr ) )
  {
    // GDAL tends to open AAIGrid as Float32 which results in lost precision
    // and confusing values shown to users, force Float64
    CPLSetConfigOption( "AAIGRID_DATATYPE", "Float64" );
  }

#if !(GDAL_VERSION_MAJOR > 2 || (GDAL_VERSION_MAJOR == 2 && GDAL_VERSION_MINOR >= 3))
  if ( !CPLGetConfigOption( "VRT_SHARED_SOURCE", nullptr ) )
  {
    // GDAL < 2.3 has issues with use of VRT in multi-threaded
    // scenarios. See https://issues.qgis.org/issues/16507 /
    // https://trac.osgeo.org/gdal/ticket/6939
    CPLSetConfigOption( "VRT_SHARED_SOURCE", "NO" );
  }
#endif

  // To get buildSupportedRasterFileFilter the provider is called with empty uri
  if ( uri.isEmpty() )
  {
    return;
  }

  mGdalDataset = nullptr;
  if ( dataset )
  {
    mGdalBaseDataset = dataset;
    initBaseDataset();
  }
  else
  {
    ( void )initIfNeeded();
  }
}

QgsGdalProvider::QgsGdalProvider( const QgsGdalProvider &other )
  : QgsRasterDataProvider( other.dataSourceUri(), QgsDataProvider::ProviderOptions() )
  , mUpdate( false )
{
  QString driverShortName;
  if ( other.mGdalBaseDataset )
  {
    driverShortName = GDALGetDriverShortName( GDALGetDatasetDriver( other.mGdalBaseDataset ) );
  }


  // The JP2OPENJPEG driver might consume too much memory on large datasets
  // so make sure to really use a single one.
  // The PostGISRaster driver internally uses a per-thread connection cache.
  // This can lead to crashes if two datasets created by the same thread are used at the same time.
  bool forceUseSameDataset = ( driverShortName.toUpper() == QStringLiteral( "JP2OPENJPEG" ) ||
                               driverShortName == QStringLiteral( "PostGISRaster" ) ||
                               CSLTestBoolean( CPLGetConfigOption( "QGIS_GDAL_FORCE_USE_SAME_DATASET", "FALSE" ) ) );

  if ( forceUseSameDataset )
  {
    ++ ( *other.mpRefCounter );
    mpRefCounter = other.mpRefCounter;
    mpMutex = other.mpMutex;
    mpLightRefCounter = new QAtomicInt( 1 );
    mHasInit = other.mHasInit;
    mValid = other.mValid;
    mGdalBaseDataset = other.mGdalBaseDataset;
    mGdalDataset = other.mGdalDataset;
  }
  else
  {

    ++ ( *other.mpLightRefCounter );

    mpRefCounter = new QAtomicInt( 1 );
    mpLightRefCounter = other.mpLightRefCounter;
    mpMutex = new QMutex( QMutex::Recursive );
    mpParent = other.mpParent;

    if ( getCachedGdalHandles( const_cast<QgsGdalProvider *>( &other ), mGdalBaseDataset, mGdalDataset ) )
    {
      QgsDebugMsgLevel( QStringLiteral( "recycling already opened dataset" ), 5 );
      mHasInit = true;
      mValid = other.mValid;
    }
    else
    {
      QgsDebugMsgLevel( QStringLiteral( "will need to open new dataset" ), 5 );
      mHasInit = false;
      mValid = false;
    }

  }

  mHasPyramids = other.mHasPyramids;
  mGdalDataType = other.mGdalDataType;
  mExtent = other.mExtent;
  mWidth = other.mWidth;
  mHeight = other.mHeight;
  mXBlockSize = other.mXBlockSize;
  mYBlockSize = other.mYBlockSize;
  memcpy( mGeoTransform, other.mGeoTransform, sizeof( mGeoTransform ) );
  mCrs = other.mCrs;
  mPyramidList = other.mPyramidList;
  mSubLayers = other.mSubLayers;
  mMaskBandExposedAsAlpha = other.mMaskBandExposedAsAlpha;
  mBandCount = other.mBandCount;
  copyBaseSettings( other );
}

QString QgsGdalProvider::dataSourceUri( bool expandAuthConfig ) const
{
  if ( expandAuthConfig && QgsDataProvider::dataSourceUri( ).contains( QLatin1String( "authcfg" ) ) )
  {
    QString uri( QgsDataProvider::dataSourceUri() );
    // Check for authcfg
    QRegularExpression authcfgRe( " authcfg='([^']+)'" );
    QRegularExpressionMatch match;
    if ( uri.contains( authcfgRe, &match ) )
    {
      uri = uri.replace( match.captured( 0 ), QString() );
      QString configId( match.captured( 1 ) );
      QStringList connectionItems;
      connectionItems << uri;
      if ( QgsApplication::authManager()->updateDataSourceUriItems( connectionItems, configId, QStringLiteral( "gdal" ) ) )
      {
        uri = connectionItems.first( );
      }
    }
    return uri;
  }
  else
  {
    return QgsDataProvider::dataSourceUri();
  }
}

QgsGdalProvider *QgsGdalProvider::clone() const
{
  return new QgsGdalProvider( *this );
}

bool QgsGdalProvider::crsFromWkt( const char *wkt )
{

  OGRSpatialReferenceH hCRS = OSRNewSpatialReference( nullptr );

  if ( OSRImportFromWkt( hCRS, ( char ** ) &wkt ) == OGRERR_NONE )
  {
    if ( OSRAutoIdentifyEPSG( hCRS ) == OGRERR_NONE )
    {
      QString authid = QStringLiteral( "%1:%2" )
                       .arg( OSRGetAuthorityName( hCRS, nullptr ),
                             OSRGetAuthorityCode( hCRS, nullptr ) );
      QgsDebugMsg( "authid recognized as " + authid );
      mCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( authid );
    }
    else
    {
      // get the proj4 text
      char *pszProj4 = nullptr;
      OSRExportToProj4( hCRS, &pszProj4 );
      QgsDebugMsg( pszProj4 );
      CPLFree( pszProj4 );

      char *pszWkt = nullptr;
      OSRExportToWkt( hCRS, &pszWkt );
      QString myWktString = QString( pszWkt );
      CPLFree( pszWkt );

      // create CRS from Wkt
      mCrs = QgsCoordinateReferenceSystem::fromWkt( myWktString );
    }
  }

  OSRRelease( hCRS );

  return mCrs.isValid();
}

bool QgsGdalProvider::getCachedGdalHandles( QgsGdalProvider *provider,
    GDALDatasetH &gdalBaseDataset,
    GDALDatasetH &gdalDataset )
{
  QMutexLocker locker( &gGdaProviderMutex );

  auto iter = mgDatasetCache.find( provider );
  if ( iter == mgDatasetCache.end() )
  {
    return false;
  }

  if ( !iter.value().isEmpty() )
  {
    DatasetPair pair = iter.value().takeFirst();
    mgDatasetCacheSize --;
    gdalBaseDataset = pair.mGdalBaseDataset;
    gdalDataset = pair.mGdalDataset;
    return true;
  }
  return false;
}

bool QgsGdalProvider::cacheGdalHandlesForLaterReuse( QgsGdalProvider *provider,
    GDALDatasetH gdalBaseDataset,
    GDALDatasetH gdalDataset )
{
  QMutexLocker locker( &gGdaProviderMutex );

  // If the cache size goes above the soft limit, try to do evict a cached
  // dataset for the provider that has the most cached entries
  if ( mgDatasetCacheSize >= MIN_THRESHOLD_FOR_CACHE_CLEANUP )
  {
    auto iter = mgDatasetCache.find( provider );
    if ( iter == mgDatasetCache.end() || iter.value().isEmpty() )
    {
      QgsGdalProvider *candidateProvider = nullptr;
      int nLargestCountOfCachedDatasets = 0;
      for ( iter = mgDatasetCache.begin(); iter != mgDatasetCache.end(); ++iter )
      {
        if ( iter.value().size() > nLargestCountOfCachedDatasets )
        {
          candidateProvider = iter.key();
          nLargestCountOfCachedDatasets = iter.value().size();
        }
      }

      Q_ASSERT( candidateProvider );
      Q_ASSERT( !mgDatasetCache[ candidateProvider ].isEmpty() );

      // If the candidate is ourselves, then do nothing
      if ( candidateProvider == provider )
        return false;

      // If the candidate provider has at least 2 cached datasets, then
      // we can evict one.
      // In the case where providers have at most one cached dataset, then
      // evict one arbitrarily
      if ( nLargestCountOfCachedDatasets >= 2 ||
           mgDatasetCacheSize >= MAX_CACHE_SIZE )
      {
        mgDatasetCacheSize --;
        DatasetPair pair = mgDatasetCache[ candidateProvider ].takeLast();
        if ( pair.mGdalBaseDataset != pair.mGdalDataset )
        {
          GDALDereferenceDataset( pair.mGdalBaseDataset );
        }
        if ( pair.mGdalDataset )
        {
          GDALClose( pair.mGdalDataset );
        }
      }
    }
    else
    {
      return false;
    }
  }

  // Add handles to the cache
  auto iter = mgDatasetCache.find( provider );
  if ( iter == mgDatasetCache.end() )
  {
    mgDatasetCache[provider] = QVector<DatasetPair>();
    iter = mgDatasetCache.find( provider );
  }

  mgDatasetCacheSize ++;
  DatasetPair pair;
  pair.mGdalBaseDataset = gdalBaseDataset;
  pair.mGdalDataset = gdalDataset;
  iter.value().push_back( pair );

  return true;
}

void QgsGdalProvider::closeCachedGdalHandlesFor( QgsGdalProvider *provider )
{
  QMutexLocker locker( &gGdaProviderMutex );
  auto iter = mgDatasetCache.find( provider );
  if ( iter != mgDatasetCache.end() )
  {
    while ( !iter.value().isEmpty() )
    {
      mgDatasetCacheSize --;
      DatasetPair pair = iter.value().takeLast();
      if ( pair.mGdalBaseDataset != pair.mGdalDataset )
      {
        GDALDereferenceDataset( pair.mGdalBaseDataset );
      }
      if ( pair.mGdalDataset )
      {
        GDALClose( pair.mGdalDataset );
      }
    }
    mgDatasetCache.erase( iter );
  }
}


QgsGdalProvider::~QgsGdalProvider()
{
  QMutexLocker locker( &gGdaProviderMutex );

  int lightRefCounter = -- ( *mpLightRefCounter );
  int refCounter = -- ( *mpRefCounter );
  if ( refCounter == 0 )
  {
    if ( mpParent && *mpParent && *mpParent != this && mGdalBaseDataset &&
         cacheGdalHandlesForLaterReuse( *mpParent, mGdalBaseDataset, mGdalDataset ) )
    {
      // do nothing
    }
    else
    {
      if ( mGdalBaseDataset != mGdalDataset )
      {
        GDALDereferenceDataset( mGdalBaseDataset );
      }
      if ( mGdalDataset )
      {
        // Check if already a PAM (persistent auxiliary metadata) file exists
        QString pamFile = dataSourceUri( true ) + QLatin1String( ".aux.xml" );
        bool pamFileAlreadyExists = QFileInfo::exists( pamFile );

        GDALClose( mGdalDataset );

        // If GDAL created a PAM file right now by using estimated metadata, delete it right away
        if ( !mStatisticsAreReliable && !pamFileAlreadyExists && QFileInfo::exists( pamFile ) )
          QFile( pamFile ).remove();
      }

      if ( mpParent && *mpParent == this )
      {
        *mpParent = nullptr;
        closeCachedGdalHandlesFor( this );
      }
    }
    delete mpMutex;
    delete mpRefCounter;
    if ( lightRefCounter == 0 )
    {
      delete mpLightRefCounter;
      delete mpParent;
    }
  }
}


void QgsGdalProvider::closeDataset()
{
  if ( !mValid )
  {
    return;
  }
  mValid = false;

  if ( mGdalBaseDataset != mGdalDataset )
  {
    GDALDereferenceDataset( mGdalBaseDataset );
  }
  mGdalBaseDataset = nullptr;

  GDALClose( mGdalDataset );
  mGdalDataset = nullptr;

  closeCachedGdalHandlesFor( this );
}

QString QgsGdalProvider::htmlMetadata()
{
  QMutexLocker locker( mpMutex );
  if ( !initIfNeeded() )
    return QString();

  QString myMetadata;

  // GDAL Driver description
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "GDAL Driver Description" ) + QStringLiteral( "</td><td>" ) + QString( GDALGetDescription( GDALGetDatasetDriver( mGdalDataset ) ) ) + QStringLiteral( "</td></tr>\n" );

  // GDAL Driver Metadata
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "GDAL Driver Metadata" ) + QStringLiteral( "</td><td>" ) + QString( GDALGetMetadataItem( GDALGetDatasetDriver( mGdalDataset ), GDAL_DMD_LONGNAME, nullptr ) ) + QStringLiteral( "</td></tr>\n" );

  // Dataset description
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Dataset Description" ) + QStringLiteral( "</td><td>" ) + QString::fromUtf8( GDALGetDescription( mGdalDataset ) ) + QStringLiteral( "</td></tr>\n" );

  // compression
  QString compression = QString( GDALGetMetadataItem( mGdalDataset, "COMPRESSION", "IMAGE_STRUCTURE" ) );
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Compression" ) + QStringLiteral( "</td><td>" ) + compression + QStringLiteral( "</td></tr>\n" );

  // Band details
  for ( int i = 1; i <= GDALGetRasterCount( mGdalDataset ); ++i )
  {
    GDALRasterBandH gdalBand = GDALGetRasterBand( mGdalDataset, i );
    char **GDALmetadata = GDALGetMetadata( gdalBand, nullptr );
    myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Band %1" ).arg( i ) + QStringLiteral( "</td><td>" );
    if ( GDALmetadata )
    {
      QStringList metadata = QgsOgrUtils::cStringListToQStringList( GDALmetadata );
      myMetadata += QgsHtmlUtils::buildBulletList( metadata );
    }

    char **GDALcategories = GDALGetRasterCategoryNames( gdalBand );

    if ( GDALcategories )
    {
      QStringList categories = QgsOgrUtils::cStringListToQStringList( GDALcategories );
      myMetadata += QgsHtmlUtils::buildBulletList( categories );
    }
    myMetadata += QStringLiteral( "</td></tr>" );
  }

  // More information
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "More information" ) + QStringLiteral( "</td><td>\n" );

  if ( mMaskBandExposedAsAlpha )
  {
    myMetadata += tr( "Mask band (exposed as alpha band)" ) + QStringLiteral( "<br />\n" );
  }

  char **GDALmetadata = GDALGetMetadata( mGdalDataset, nullptr );
  if ( GDALmetadata )
  {
    QStringList metadata = QgsOgrUtils::cStringListToQStringList( GDALmetadata );
    myMetadata += QgsHtmlUtils::buildBulletList( metadata );
  }

  //just use the first band
  if ( GDALGetRasterCount( mGdalDataset ) > 0 )
  {
    GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, 1 );
    if ( GDALGetOverviewCount( myGdalBand ) > 0 )
    {
      int myOverviewInt;
      for ( myOverviewInt = 0; myOverviewInt < GDALGetOverviewCount( myGdalBand ); myOverviewInt++ )
      {
        GDALRasterBandH myOverview;
        myOverview = GDALGetOverview( myGdalBand, myOverviewInt );
        QStringList metadata;
        metadata.append( QStringLiteral( "X : " ) + QString::number( GDALGetRasterBandXSize( myOverview ) ) );
        metadata.append( QStringLiteral( "Y : " ) + QString::number( GDALGetRasterBandYSize( myOverview ) ) );
        myMetadata += QgsHtmlUtils::buildBulletList( metadata );
      }
    }
  }

  // End more information
  myMetadata += QStringLiteral( "</td></tr>\n" );

  // Dimensions
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Dimensions" ) + QStringLiteral( "</td><td>" );
  myMetadata += tr( "X: %1 Y: %2 Bands: %3" )
                .arg( GDALGetRasterXSize( mGdalDataset ) )
                .arg( GDALGetRasterYSize( mGdalDataset ) )
                .arg( GDALGetRasterCount( mGdalDataset ) );
  myMetadata += QStringLiteral( "</td></tr>\n" );

  if ( GDALGetGeoTransform( mGdalDataset, mGeoTransform ) != CE_None )
  {
    // if the raster does not have a valid transform we need to use
    // a pixel size of (1,-1), but GDAL returns (1,1)
    mGeoTransform[5] = -1;
  }
  else
  {
    // Origin
    myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Origin" ) + QStringLiteral( "</td><td>" ) + QString::number( mGeoTransform[0] ) + QStringLiteral( "," ) + QString::number( mGeoTransform[3] ) + QStringLiteral( "</td></tr>\n" );

    // Pixel size
    myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Pixel Size" ) + QStringLiteral( "</td><td>" ) + QString::number( mGeoTransform[1], 'g', 19 ) + QStringLiteral( "," ) + QString::number( mGeoTransform[5], 'g', 19 ) + QStringLiteral( "</td></tr>\n" );
  }

  return myMetadata;
}


QgsRasterBlock *QgsGdalProvider::block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback )
{
  QgsRasterBlock *block = new QgsRasterBlock( dataType( bandNo ), width, height );
  if ( !initIfNeeded() )
    return block;
  if ( sourceHasNoDataValue( bandNo ) && useSourceNoDataValue( bandNo ) )
  {
    block->setNoDataValue( sourceNoDataValue( bandNo ) );
  }

  if ( block->isEmpty() )
  {
    return block;
  }

  if ( !mExtent.intersects( extent ) )
  {
    // the requested extent is completely outside of the raster's extent - nothing to do
    block->setIsNoData();
    return block;
  }

  if ( !mExtent.contains( extent ) )
  {
    QRect subRect = QgsRasterBlock::subRect( extent, width, height, mExtent );
    block->setIsNoDataExcept( subRect );
  }
  readBlock( bandNo, extent, width, height, block->bits(), feedback );
  // apply scale and offset
  block->applyScaleOffset( bandScale( bandNo ), bandOffset( bandNo ) );
  block->applyNoDataValues( userNoDataValues( bandNo ) );
  return block;
}

void QgsGdalProvider::readBlock( int bandNo, int xBlock, int yBlock, void *data )
{
  QMutexLocker locker( mpMutex );
  if ( !initIfNeeded() )
    return;

  // TODO!!!: Check data alignment!!! May it happen that nearest value which
  // is not nearest is assigned to an output cell???


  //QgsDebugMsg( "yBlock = "  + QString::number( yBlock ) );

  GDALRasterBandH myGdalBand = getBand( bandNo );
  //GDALReadBlock( myGdalBand, xBlock, yBlock, block );

  // We have to read with correct data type consistent with other readBlock functions
  int xOff = xBlock * mXBlockSize;
  int yOff = yBlock * mYBlockSize;
  gdalRasterIO( myGdalBand, GF_Read, xOff, yOff, mXBlockSize, mYBlockSize, data, mXBlockSize, mYBlockSize, ( GDALDataType ) mGdalDataType.at( bandNo - 1 ), 0, 0 );
}

void QgsGdalProvider::readBlock( int bandNo, QgsRectangle  const &extent, int pixelWidth, int pixelHeight, void *data, QgsRasterBlockFeedback *feedback )
{
  QMutexLocker locker( mpMutex );
  if ( !initIfNeeded() )
    return;

  QgsDebugMsgLevel( "pixelWidth = "  + QString::number( pixelWidth ), 5 );
  QgsDebugMsgLevel( "pixelHeight = "  + QString::number( pixelHeight ), 5 );
  QgsDebugMsgLevel( "extent: " + extent.toString(), 5 );

  for ( int i = 0; i < 6; i++ )
  {
    QgsDebugMsgLevel( QStringLiteral( "transform : %1" ).arg( mGeoTransform[i] ), 5 );
  }

  size_t dataSize = static_cast<size_t>( dataTypeSize( bandNo ) );

  // moved to block()
#if 0
  if ( !mExtent.contains( extent ) )
  {
    // fill with null values
    QByteArray ba = QgsRasterBlock::valueBytes( dataType( bandNo ), noDataValue( bandNo ) );
    char *nodata = ba.data();
    char *block = ( char * ) block;
    for ( int i = 0; i < pixelWidth * pixelHeight; i++ )
    {
      memcpy( block, nodata, dataSize );
      block += dataSize;
    }
  }
#endif

  QgsRectangle rasterExtent = extent.intersect( mExtent );
  if ( rasterExtent.isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "draw request outside view extent." ) );
    return;
  }
  QgsDebugMsgLevel( "extent: " + mExtent.toString(), 5 );
  QgsDebugMsgLevel( "rasterExtent: " + rasterExtent.toString(), 5 );

  double xRes = extent.width() / pixelWidth;
  double yRes = extent.height() / pixelHeight;

  // Find top, bottom rows and left, right column the raster extent covers
  // These are limits in target grid space
#if 0
  int top = 0;
  int bottom = pixelHeight - 1;
  int left = 0;
  int right = pixelWidth - 1;

  if ( myRasterExtent.yMaximum() < extent.yMaximum() )
  {
    top = std::round( ( extent.yMaximum() - myRasterExtent.yMaximum() ) / yRes );
  }
  if ( myRasterExtent.yMinimum() > extent.yMinimum() )
  {
    bottom = std::round( ( extent.yMaximum() - myRasterExtent.yMinimum() ) / yRes ) - 1;
  }

  if ( myRasterExtent.xMinimum() > extent.xMinimum() )
  {
    left = std::round( ( myRasterExtent.xMinimum() - extent.xMinimum() ) / xRes );
  }
  if ( myRasterExtent.xMaximum() < extent.xMaximum() )
  {
    right = std::round( ( myRasterExtent.xMaximum() - extent.xMinimum() ) / xRes ) - 1;
  }
#endif
  QRect subRect = QgsRasterBlock::subRect( extent, pixelWidth, pixelHeight, rasterExtent );
  int top = subRect.top();
  int bottom = subRect.bottom();
  int left = subRect.left();
  int right = subRect.right();
  QgsDebugMsgLevel( QStringLiteral( "top = %1 bottom = %2 left = %3 right = %4" ).arg( top ).arg( bottom ).arg( left ).arg( right ), 5 );


  // We want to avoid another resampling, so we read data approximately with
  // the same resolution as requested and exactly the width/height we need.

  // Calculate rows/cols limits in raster grid space

  // Set readable names
  double srcXRes = mGeoTransform[1];
  double srcYRes = mGeoTransform[5]; // may be negative?
  QgsDebugMsgLevel( QStringLiteral( "xRes = %1 yRes = %2 srcXRes = %3 srcYRes = %4" ).arg( xRes ).arg( yRes ).arg( srcXRes ).arg( srcYRes ), 5 );

  // target size in pizels
  int width = right - left + 1;
  int height = bottom - top + 1;

  int srcLeft = 0; // source raster x offset
  int srcTop = 0; // source raster x offset
  int srcBottom = ySize() - 1;
  int srcRight = xSize() - 1;

  // Note: original approach for xRes < srcXRes || yRes < std::fabs( srcYRes ) was to avoid
  // second resampling and read with GDALRasterIO to another temporary data block
  // extended to fit src grid. The problem was that with src resolution much bigger
  // than dst res, the target could become very large
  // in theory it was going to infinity when zooming in ...

  // Note: original approach for xRes > srcXRes, yRes > srcYRes was to read directly with GDALRasterIO
  // but we would face this problem:
  // If the edge of the source is greater than the edge of destination:
  // src:        | | | | | | | | |
  // dst:     |      |     |     |
  // We have 2 options for resampling:
  //  a) 'Stretch' the src and align the start edge of src to the start edge of dst.
  //     That means however, that to the target cells may be assigned values of source
  //     which are not nearest to the center of dst cells. Usually probably not a problem
  //     but we are not precise. The shift is in maximum ... TODO
  //  b) We could cut the first destination column and left only the second one which is
  //     completely covered by src. No (significant) stretching is applied in that
  //     case, but the first column may be rendered as without values event if its center
  //     is covered by src column. That could result in wrongly rendered (missing) edges
  //     which could be easily noticed by user

  // Because of problems mentioned above we read to another temporary block and do i
  // another resampling here which appeares to be quite fast

  // Get necessary src extent aligned to src resolution
  if ( mExtent.xMinimum() < rasterExtent.xMinimum() )
  {
    srcLeft = static_cast<int>( std::floor( ( rasterExtent.xMinimum() - mExtent.xMinimum() ) / srcXRes ) );
  }
  if ( mExtent.xMaximum() > rasterExtent.xMaximum() )
  {
    srcRight = static_cast<int>( std::floor( ( rasterExtent.xMaximum() - mExtent.xMinimum() ) / srcXRes ) );
  }

  // GDAL states that mGeoTransform[3] is top, may it also be bottom and mGeoTransform[5] positive?
  if ( mExtent.yMaximum() > rasterExtent.yMaximum() )
  {
    srcTop = static_cast<int>( std::floor( -1. * ( mExtent.yMaximum() - rasterExtent.yMaximum() ) / srcYRes ) );
  }
  if ( mExtent.yMinimum() < rasterExtent.yMinimum() )
  {
    srcBottom = static_cast<int>( std::floor( -1. * ( mExtent.yMaximum() - rasterExtent.yMinimum() ) / srcYRes ) );
  }

  QgsDebugMsgLevel( QStringLiteral( "srcTop = %1 srcBottom = %2 srcLeft = %3 srcRight = %4" ).arg( srcTop ).arg( srcBottom ).arg( srcLeft ).arg( srcRight ), 5 );

  int srcWidth = srcRight - srcLeft + 1;
  int srcHeight = srcBottom - srcTop + 1;

  QgsDebugMsgLevel( QStringLiteral( "width = %1 height = %2 srcWidth = %3 srcHeight = %4" ).arg( width ).arg( height ).arg( srcWidth ).arg( srcHeight ), 5 );

  int tmpWidth = srcWidth;
  int tmpHeight = srcHeight;

  if ( xRes > srcXRes )
  {
    tmpWidth = static_cast<int>( std::round( srcWidth * srcXRes / xRes ) );
  }
  if ( yRes > std::fabs( srcYRes ) )
  {
    tmpHeight = static_cast<int>( std::round( -1.*srcHeight * srcYRes / yRes ) );
  }

  double tmpXMin = mExtent.xMinimum() + srcLeft * srcXRes;
  double tmpYMax = mExtent.yMaximum() + srcTop * srcYRes;
  QgsDebugMsgLevel( QStringLiteral( "tmpXMin = %1 tmpYMax = %2 tmpWidth = %3 tmpHeight = %4" ).arg( tmpXMin ).arg( tmpYMax ).arg( tmpWidth ).arg( tmpHeight ), 5 );

  // Allocate temporary block
  size_t bufferSize = dataSize * static_cast<size_t>( tmpWidth ) * static_cast<size_t>( tmpHeight );
#ifdef Q_PROCESSOR_X86_32
  // Safety check for 32 bit systems
  qint64 _buffer_size = dataSize * static_cast<qint64>( tmpWidth ) * static_cast<qint64>( tmpHeight );
  if ( _buffer_size != static_cast<qint64>( bufferSize ) )
  {
    QgsDebugMsg( QStringLiteral( "Integer overflow calculating buffer size on a 32 bit system." ) );
    return;
  }
#endif
  char *tmpBlock = static_cast<char *>( qgsMalloc( bufferSize ) );
  if ( ! tmpBlock )
  {
    QgsDebugMsgLevel( QStringLiteral( "Couldn't allocate temporary buffer of %1 bytes" ).arg( dataSize * tmpWidth * tmpHeight ), 5 );
    return;
  }
  GDALRasterBandH gdalBand = getBand( bandNo );
  GDALDataType type = static_cast<GDALDataType>( mGdalDataType.at( bandNo - 1 ) );
  CPLErrorReset();

  CPLErr err = gdalRasterIO( gdalBand, GF_Read,
                             srcLeft, srcTop, srcWidth, srcHeight,
                             static_cast<void *>( tmpBlock ),
                             tmpWidth, tmpHeight, type,
                             0, 0, feedback );

  if ( err != CPLE_None )
  {
    QgsLogger::warning( "RasterIO error: " + QString::fromUtf8( CPLGetLastErrorMsg() ) );
    qgsFree( tmpBlock );
    return;
  }

  double tmpXRes = srcWidth * srcXRes / tmpWidth;
  double tmpYRes = srcHeight * srcYRes / tmpHeight; // negative

  double y = rasterExtent.yMaximum() - 0.5 * yRes;
  for ( int row = 0; row < height; row++ )
  {
    int tmpRow = static_cast<int>( std::floor( -1. * ( tmpYMax - y ) / tmpYRes ) );

    char *srcRowBlock = tmpBlock + dataSize * tmpRow * tmpWidth;
    char *dstRowBlock = ( char * )data + dataSize * ( top + row ) * pixelWidth;

    double x = ( rasterExtent.xMinimum() + 0.5 * xRes - tmpXMin ) / tmpXRes; // cell center
    double increment = xRes / tmpXRes;

    char *dst = dstRowBlock + dataSize * left;
    char *src = srcRowBlock;
    int tmpCol = 0;
    int lastCol = 0;
    for ( int col = 0; col < width; ++col )
    {
      // std::floor() is quite slow! Use just cast to int.
      tmpCol = static_cast<int>( x );
      if ( tmpCol > lastCol )
      {
        src += ( tmpCol - lastCol ) * dataSize;
        lastCol = tmpCol;
      }
      memcpy( dst, src, dataSize );
      dst += dataSize;
      x += increment;
    }
    y -= yRes;
  }

  qgsFree( tmpBlock );
}

/**
 * \param bandNumber the number of the band for which you want a color table
 * \param list a pointer the object that will hold the color table
 * \return true of a color table was able to be read, false otherwise
 */
QList<QgsColorRampShader::ColorRampItem> QgsGdalProvider::colorTable( int bandNumber )const
{
  QMutexLocker locker( mpMutex );
  if ( !const_cast<QgsGdalProvider *>( this )->initIfNeeded() )
    return QList<QgsColorRampShader::ColorRampItem>();
  return QgsGdalProviderBase::colorTable( mGdalDataset, bandNumber );
}

QgsCoordinateReferenceSystem QgsGdalProvider::crs() const
{
  return mCrs;
}

QgsRectangle QgsGdalProvider::extent() const
{
  //TODO
  //mExtent = QgsGdal::extent( mGisdbase, mLocation, mMapset, mMapName, QgsGdal::Raster );
  return mExtent;
}

// this is only called once when statistics are calculated
// TODO
int QgsGdalProvider::xBlockSize() const
{
  return mXBlockSize;
}
int QgsGdalProvider::yBlockSize() const
{
  return mYBlockSize;
}

int QgsGdalProvider::xSize() const { return mWidth; }
int QgsGdalProvider::ySize() const { return mHeight; }

QString QgsGdalProvider::generateBandName( int bandNumber ) const
{
  QMutexLocker locker( mpMutex );
  if ( !const_cast<QgsGdalProvider *>( this )->initIfNeeded() )
    return QString();

  if ( strcmp( GDALGetDriverShortName( GDALGetDatasetDriver( mGdalDataset ) ), "netCDF" ) == 0 || strcmp( GDALGetDriverShortName( GDALGetDatasetDriver( mGdalDataset ) ), "GTiff" ) == 0 )
  {
    char **GDALmetadata = GDALGetMetadata( mGdalDataset, nullptr );
    if ( GDALmetadata )
    {
      QStringList metadata = QgsOgrUtils::cStringListToQStringList( GDALmetadata );
      QStringList dimExtraValues;
      QMap<QString, QString> unitsMap;
      for ( QStringList::const_iterator i = metadata.constBegin(); i != metadata.constEnd(); ++i )
      {
        QString val( *i );
        if ( !val.startsWith( QLatin1String( "NETCDF_DIM_EXTRA" ) ) && !val.startsWith( QLatin1String( "GTIFF_DIM_EXTRA" ) ) && !val.contains( QLatin1String( "#units=" ) ) )
          continue;
        QStringList values = val.split( '=' );
        val = values.at( 1 );
        if ( values.at( 0 ) == QLatin1String( "NETCDF_DIM_EXTRA" ) || values.at( 0 ) == QLatin1String( "GTIFF_DIM_EXTRA" ) )
        {
          dimExtraValues = val.replace( '{', QString() ).replace( '}', QString() ).split( ',' );
          //http://qt-project.org/doc/qt-4.8/qregexp.html#capturedTexts
        }
        else
        {
          unitsMap[values.at( 0 ).split( '#' ).at( 0 )] = val;
        }
      }
      if ( !dimExtraValues.isEmpty() )
      {
        QStringList bandNameValues;
        GDALRasterBandH gdalBand = GDALGetRasterBand( mGdalDataset, bandNumber );
        GDALmetadata = GDALGetMetadata( gdalBand, nullptr );
        if ( GDALmetadata )
        {
          metadata = QgsOgrUtils::cStringListToQStringList( GDALmetadata );
          for ( QStringList::const_iterator i = metadata.constBegin(); i != metadata.constEnd(); ++i )
          {
            QString val( *i );
            if ( !val.startsWith( QLatin1String( "NETCDF_DIM_" ) ) && !val.startsWith( QLatin1String( "GTIFF_DIM_" ) ) )
              continue;
            QStringList values = val.split( '=' );
            for ( QStringList::const_iterator j = dimExtraValues.constBegin(); j != dimExtraValues.constEnd(); ++j )
            {
              QString dim = ( *j );
              if ( values.at( 0 ) != "NETCDF_DIM_" + dim && values.at( 0 ) != "GTIFF_DIM_" + dim )
                continue;
              if ( unitsMap.contains( dim ) && !unitsMap[dim].isEmpty() && unitsMap[dim] != QLatin1String( "none" ) )
                bandNameValues.append( dim + '=' + values.at( 1 ) + " (" + unitsMap[dim] + ')' );
              else
                bandNameValues.append( dim + '=' + values.at( 1 ) );
            }
          }
        }
        if ( !bandNameValues.isEmpty() )
        {
          return tr( "Band" ) + QStringLiteral( " %1: %2" ).arg( bandNumber, 1 + ( int ) std::log10( ( float ) bandCount() ), 10, QChar( '0' ) ).arg( bandNameValues.join( QStringLiteral( " / " ) ) );
        }
      }
    }
  }
  QString generatedBandName = QgsRasterDataProvider::generateBandName( bandNumber );
  GDALRasterBandH myGdalBand = getBand( bandNumber );
  QString gdalBandName( GDALGetDescription( myGdalBand ) );
  if ( !gdalBandName.isEmpty() )
  {
    return generatedBandName + QStringLiteral( ": " ) + gdalBandName;
  }
  return generatedBandName;
}

QgsRasterIdentifyResult QgsGdalProvider::identify( const QgsPointXY &point, QgsRaster::IdentifyFormat format, const QgsRectangle &boundingBox, int width, int height, int /*dpi*/ )
{
  QMutexLocker locker( mpMutex );
  if ( !initIfNeeded() )
    return QgsRasterIdentifyResult( ERR( tr( "Cannot read data" ) ) );

  QgsDebugMsgLevel( QStringLiteral( "thePoint = %1 %2" ).arg( point.x(), 0, 'g', 10 ).arg( point.y(), 0, 'g', 10 ), 3 );

  QMap<int, QVariant> results;

  if ( format != QgsRaster::IdentifyFormatValue )
  {
    return QgsRasterIdentifyResult( ERR( tr( "Format not supported" ) ) );
  }

  if ( !extent().contains( point ) )
  {
    // Outside the raster
    for ( int bandNo = 1; bandNo <= bandCount(); bandNo++ )
    {
      results.insert( bandNo, QVariant() ); // null QVariant represents no data
    }
    return QgsRasterIdentifyResult( QgsRaster::IdentifyFormatValue, results );
  }

  QgsRectangle finalExtent = boundingBox;
  if ( finalExtent.isEmpty() )
    finalExtent = extent();

  QgsDebugMsgLevel( QStringLiteral( "myExtent = %1 " ).arg( finalExtent.toString() ), 3 );

  if ( width == 0 )
    width = xSize();
  if ( height == 0 )
    height = ySize();

  QgsDebugMsgLevel( QStringLiteral( "theWidth = %1 height = %2" ).arg( width ).arg( height ), 3 );

  // Calculate the row / column where the point falls
  double xres = ( finalExtent.width() ) / width;
  double yres = ( finalExtent.height() ) / height;

  // Offset, not the cell index -> std::floor
  int col = static_cast< int >( std::floor( ( point.x() - finalExtent.xMinimum() ) / xres ) );
  int row = static_cast< int >( std::floor( ( finalExtent.yMaximum() - point.y() ) / yres ) );

  QgsDebugMsgLevel( QStringLiteral( "row = %1 col = %2" ).arg( row ).arg( col ), 3 );

  // QgsDebugMsg( "row = " + QString::number( row ) + " col = " + QString::number( col ) );

  int r = 0;
  int c = 0;
  int w = 1;
  int h = 1;

  double xMin = finalExtent.xMinimum() + col * xres;
  double xMax = xMin + xres * w;
  double yMax = finalExtent.yMaximum() - row * yres;
  double yMin = yMax - yres * h;
  QgsRectangle pixelExtent( xMin, yMin, xMax, yMax );

  for ( int i = 1; i <= bandCount(); i++ )
  {
    std::unique_ptr< QgsRasterBlock > bandBlock( block( i, pixelExtent, w, h ) );

    if ( !bandBlock )
    {
      return QgsRasterIdentifyResult( ERR( tr( "Cannot read data" ) ) );
    }

    double value = bandBlock->value( r, c );

    if ( ( sourceHasNoDataValue( i ) && useSourceNoDataValue( i ) &&
           ( std::isnan( value ) || qgsDoubleNear( value, sourceNoDataValue( i ) ) ) ) ||
         ( QgsRasterRange::contains( value, userNoDataValues( i ) ) ) )
    {
      results.insert( i, QVariant() ); // null QVariant represents no data
    }
    else
    {
      if ( sourceDataType( i ) == Qgis::Float32 )
      {
        // Insert a float QVariant so that QgsMapToolIdentify::identifyRasterLayer()
        // can print a string without an excessive precision
        results.insert( i, static_cast<float>( value ) );
      }
      else
        results.insert( i, value );
    }
  }
  return QgsRasterIdentifyResult( QgsRaster::IdentifyFormatValue, results );
}

bool QgsGdalProvider::worldToPixel( double x, double y, int &col, int &row ) const
{
  /*
   * This set of equations solves
   * Xgeo = GT(0) + XpixelGT(1) + YlineGT(2)
   * Ygeo = GT(3) + XpixelGT(4) + YlineGT(5)
   * when Xgeo, Ygeo are given
   */
  double div = ( mGeoTransform[2] * mGeoTransform[4] - mGeoTransform[1] * mGeoTransform[5] );
  if ( div < 2 * std::numeric_limits<double>::epsilon() )
    return false;
  double doubleCol = -( mGeoTransform[2] * ( mGeoTransform[3] - y ) + mGeoTransform[5] * ( x - mGeoTransform[0] ) ) / div;
  double doubleRow = ( mGeoTransform[1] * ( mGeoTransform[3] - y ) + mGeoTransform[4] * ( x - mGeoTransform[0] ) ) / div;
  // note: we truncate here, not round, otherwise values will be 0.5 pixels off
  col = static_cast< int >( doubleCol );
  row = static_cast< int >( doubleRow );
  return true;
};

double QgsGdalProvider::sample( const QgsPointXY &point, int band, bool *ok, const QgsRectangle &, int, int, int )
{
  if ( ok )
    *ok = false;

  QMutexLocker locker( mpMutex );
  if ( !initIfNeeded() )
    return std::numeric_limits<double>::quiet_NaN();

  if ( !extent().contains( point ) )
  {
    // Outside the raster
    return std::numeric_limits<double>::quiet_NaN();
  }

  GDALRasterBandH hBand = GDALGetRasterBand( mGdalDataset, band );
  if ( !hBand )
    return std::numeric_limits<double>::quiet_NaN();

  int row;
  int col;
  if ( !worldToPixel( point.x(), point.y(), col, row ) )
    return std::numeric_limits<double>::quiet_NaN();

  float value = 0;
  CPLErr err = GDALRasterIO( hBand, GF_Read, col, row, 1, 1,
                             &value, 1, 1, GDT_Float32, 0, 0 );
  if ( err != CE_None )
    return std::numeric_limits<double>::quiet_NaN();

  if ( ( sourceHasNoDataValue( band ) && useSourceNoDataValue( band ) &&
         ( std::isnan( value ) || qgsDoubleNear( static_cast< double >( value ), sourceNoDataValue( band ) ) ) ) ||
       ( QgsRasterRange::contains( static_cast< double >( value ), userNoDataValues( band ) ) ) )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  if ( ok )
    *ok = true;

  return static_cast< double >( value ) * bandScale( band ) + bandOffset( band );
}

int QgsGdalProvider::capabilities() const
{
  QMutexLocker locker( mpMutex );
  if ( !const_cast<QgsGdalProvider *>( this )->initIfNeeded() )
    return 0;

  int capability = QgsRasterDataProvider::Identify
                   | QgsRasterDataProvider::IdentifyValue
                   | QgsRasterDataProvider::Size
                   | QgsRasterDataProvider::BuildPyramids
                   | QgsRasterDataProvider::Create
                   | QgsRasterDataProvider::Remove;
  GDALDriverH myDriver = GDALGetDatasetDriver( mGdalDataset );
  QString name = GDALGetDriverShortName( myDriver );
  QgsDebugMsg( "driver short name = " + name );
  if ( name != QLatin1String( "WMS" ) )
  {
    capability |= QgsRasterDataProvider::Size;
  }
  return capability;
}

Qgis::DataType QgsGdalProvider::sourceDataType( int bandNo ) const
{
  QMutexLocker locker( mpMutex );
  if ( !const_cast<QgsGdalProvider *>( this )->initIfNeeded() )
    return dataTypeFromGdal( GDT_Byte );

  if ( mMaskBandExposedAsAlpha && bandNo == GDALGetRasterCount( mGdalDataset ) + 1 )
    return dataTypeFromGdal( GDT_Byte );

  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, bandNo );
  GDALDataType myGdalDataType = GDALGetRasterDataType( myGdalBand );
  Qgis::DataType myDataType = dataTypeFromGdal( myGdalDataType );

  // define if the band has scale and offset to apply
  double myScale = bandScale( bandNo );
  double myOffset = bandOffset( bandNo );
  if ( myScale != 1.0 || myOffset != 0.0 )
  {
    // if the band has scale or offset to apply change dataType
    switch ( myDataType )
    {
      case Qgis::UnknownDataType:
      case Qgis::ARGB32:
      case Qgis::ARGB32_Premultiplied:
        return myDataType;
      case Qgis::Byte:
      case Qgis::UInt16:
      case Qgis::Int16:
      case Qgis::UInt32:
      case Qgis::Int32:
      case Qgis::Float32:
      case Qgis::CInt16:
        myDataType = Qgis::Float32;
        break;
      case Qgis::Float64:
      case Qgis::CInt32:
      case Qgis::CFloat32:
        myDataType = Qgis::Float64;
        break;
      case Qgis::CFloat64:
        return myDataType;
    }
  }
  return myDataType;
}

Qgis::DataType QgsGdalProvider::dataType( int bandNo ) const
{
  if ( mMaskBandExposedAsAlpha && bandNo == mBandCount )
    return dataTypeFromGdal( GDT_Byte );

  if ( bandNo <= 0 || bandNo > mGdalDataType.count() ) return Qgis::UnknownDataType;

  return dataTypeFromGdal( mGdalDataType[bandNo - 1] );
}

double QgsGdalProvider::bandScale( int bandNo ) const
{
  QMutexLocker locker( mpMutex );
  if ( !const_cast<QgsGdalProvider *>( this )->initIfNeeded() )
    return 1.0;

  GDALRasterBandH myGdalBand = getBand( bandNo );
  int bGotScale;
  double myScale = GDALGetRasterScale( myGdalBand, &bGotScale );

  // if scale==0, ignore both scale and offset
  if ( bGotScale && !qgsDoubleNear( myScale, 0.0 ) )
    return myScale;
  else
    return 1.0;
}

double QgsGdalProvider::bandOffset( int bandNo ) const
{
  QMutexLocker locker( mpMutex );
  if ( !const_cast<QgsGdalProvider *>( this )->initIfNeeded() )
    return 0.0;

  GDALRasterBandH myGdalBand = getBand( bandNo );

  // if scale==0, ignore both scale and offset
  int bGotScale;
  double myScale = GDALGetRasterScale( myGdalBand, &bGotScale );
  if ( bGotScale && qgsDoubleNear( myScale, 0.0 ) )
    return 0.0;

  int bGotOffset;
  double myOffset = GDALGetRasterOffset( myGdalBand, &bGotOffset );
  if ( bGotOffset )
    return myOffset;
  else
    return 0.0;
}

int QgsGdalProvider::bandCount() const
{
  return mBandCount;
}

int QgsGdalProvider::colorInterpretation( int bandNo ) const
{
  QMutexLocker locker( mpMutex );
  if ( !const_cast<QgsGdalProvider *>( this )->initIfNeeded() )
    return colorInterpretationFromGdal( GCI_Undefined );

  if ( mMaskBandExposedAsAlpha && bandNo == GDALGetRasterCount( mGdalDataset ) + 1 )
    return colorInterpretationFromGdal( GCI_AlphaBand );
  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, bandNo );
  return colorInterpretationFromGdal( GDALGetRasterColorInterpretation( myGdalBand ) );
}

bool QgsGdalProvider::isValid() const
{
  QgsDebugMsg( QStringLiteral( "valid = %1" ).arg( mValid ) );
  return mValid;
}

QString QgsGdalProvider::lastErrorTitle()
{
  return QStringLiteral( "Not implemented" );
}

QString QgsGdalProvider::lastError()
{
  return QStringLiteral( "Not implemented" );
}

QString QgsGdalProvider::name() const
{
  return PROVIDER_KEY;
}

QString QgsGdalProvider::description() const
{
  return PROVIDER_DESCRIPTION;
}

// This is used also by global isValidRasterFileName
QStringList QgsGdalProvider::subLayers( GDALDatasetH dataset )
{
  QStringList subLayers;

  if ( !dataset )
  {
    QgsDebugMsg( QStringLiteral( "dataset is nullptr" ) );
    return subLayers;
  }

  char **metadata = GDALGetMetadata( dataset, "SUBDATASETS" );

  if ( metadata )
  {
    for ( int i = 0; metadata[i]; i++ )
    {
      QString layer = QString::fromUtf8( metadata[i] );
      int pos = layer.indexOf( QLatin1String( "_NAME=" ) );
      if ( pos >= 0 )
      {
        subLayers << layer.mid( pos + 6 );
      }
    }
  }

  if ( !subLayers.isEmpty() )
  {
    QgsDebugMsg( "sublayers:\n  " + subLayers.join( "\n  " ) );
  }

  return subLayers;
}

bool QgsGdalProvider::hasHistogram( int bandNo,
                                    int binCount,
                                    double minimum, double maximum,
                                    const QgsRectangle &boundingBox,
                                    int sampleSize,
                                    bool includeOutOfRange )
{
  QMutexLocker locker( mpMutex );
  if ( !initIfNeeded() )
    return false;

  QgsDebugMsg( QStringLiteral( "theBandNo = %1 binCount = %2 minimum = %3 maximum = %4 sampleSize = %5" ).arg( bandNo ).arg( binCount ).arg( minimum ).arg( maximum ).arg( sampleSize ) );

  // First check if cached in mHistograms
  if ( QgsRasterDataProvider::hasHistogram( bandNo, binCount, minimum, maximum, boundingBox, sampleSize, includeOutOfRange ) )
  {
    return true;
  }

  QgsRasterHistogram myHistogram;
  initHistogram( myHistogram, bandNo, binCount, minimum, maximum, boundingBox, sampleSize, includeOutOfRange );

  // If not cached, check if supported by GDAL
  if ( myHistogram.extent != extent() )
  {
    QgsDebugMsg( QStringLiteral( "Not supported by GDAL." ) );
    return false;
  }

  if ( ( sourceHasNoDataValue( bandNo ) && !useSourceNoDataValue( bandNo ) ) ||
       !userNoDataValues( bandNo ).isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "Custom no data values -> GDAL histogram not sufficient." ) );
    return false;
  }

  QgsDebugMsg( QStringLiteral( "Looking for GDAL histogram" ) );

  GDALRasterBandH myGdalBand = getBand( bandNo );
  if ( ! myGdalBand )
  {
    return false;
  }

  // get default histogram with force=false to see if there is a cached histo
  double myMinVal, myMaxVal;
  int myBinCount;

  GUIntBig *myHistogramArray = nullptr;
  CPLErr myError = GDALGetDefaultHistogramEx( myGdalBand, &myMinVal, &myMaxVal,
                   &myBinCount, &myHistogramArray, false,
                   nullptr, nullptr );

  if ( myHistogramArray )
    VSIFree( myHistogramArray ); // use VSIFree because allocated by GDAL

  // if there was any error/warning assume the histogram is not valid or non-existent
  if ( myError != CE_None )
  {
    QgsDebugMsg( QStringLiteral( "Cannot get default GDAL histogram" ) );
    return false;
  }

  // This is fragile
  double myExpectedMinVal = myHistogram.minimum;
  double myExpectedMaxVal = myHistogram.maximum;

  double dfHalfBucket = ( myExpectedMaxVal - myExpectedMinVal ) / ( 2 * myHistogram.binCount );
  myExpectedMinVal -= dfHalfBucket;
  myExpectedMaxVal += dfHalfBucket;

  // min/max are stored as text in aux file => use threshold
  if ( myBinCount != myHistogram.binCount ||
       std::fabs( myMinVal - myExpectedMinVal ) > std::fabs( myExpectedMinVal ) / 10e6 ||
       std::fabs( myMaxVal - myExpectedMaxVal ) > std::fabs( myExpectedMaxVal ) / 10e6 )
  {
    QgsDebugMsg( QStringLiteral( "Params do not match binCount: %1 x %2, minVal: %3 x %4, maxVal: %5 x %6" ).arg( myBinCount ).arg( myHistogram.binCount ).arg( myMinVal ).arg( myExpectedMinVal ).arg( myMaxVal ).arg( myExpectedMaxVal ) );
    return false;
  }

  QgsDebugMsg( QStringLiteral( "GDAL has cached histogram" ) );

  // This should be enough, possible call to histogram() should retrieve the histogram cached in GDAL

  return true;
}

QgsRasterHistogram QgsGdalProvider::histogram( int bandNo,
    int binCount,
    double minimum, double maximum,
    const QgsRectangle &boundingBox,
    int sampleSize,
    bool includeOutOfRange, QgsRasterBlockFeedback *feedback )
{
  QMutexLocker locker( mpMutex );
  if ( !initIfNeeded() )
    return QgsRasterHistogram();

  QgsDebugMsg( QStringLiteral( "theBandNo = %1 binCount = %2 minimum = %3 maximum = %4 sampleSize = %5" ).arg( bandNo ).arg( binCount ).arg( minimum ).arg( maximum ).arg( sampleSize ) );

  QgsRasterHistogram myHistogram;
  initHistogram( myHistogram, bandNo, binCount, minimum, maximum, boundingBox, sampleSize, includeOutOfRange );

  // Find cached
  Q_FOREACH ( const QgsRasterHistogram &histogram, mHistograms )
  {
    if ( histogram == myHistogram )
    {
      QgsDebugMsg( QStringLiteral( "Using cached histogram." ) );
      return histogram;
    }
  }

  if ( ( sourceHasNoDataValue( bandNo ) && !useSourceNoDataValue( bandNo ) ) ||
       !userNoDataValues( bandNo ).isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "Custom no data values, using generic histogram." ) );
    return QgsRasterDataProvider::histogram( bandNo, binCount, minimum, maximum, boundingBox, sampleSize, includeOutOfRange, feedback );
  }

  if ( myHistogram.extent != extent() )
  {
    QgsDebugMsg( QStringLiteral( "Not full extent, using generic histogram." ) );
    return QgsRasterDataProvider::histogram( bandNo, binCount, minimum, maximum, boundingBox, sampleSize, includeOutOfRange, feedback );
  }

  QgsDebugMsg( QStringLiteral( "Computing GDAL histogram" ) );

  GDALRasterBandH myGdalBand = getBand( bandNo );

  int bApproxOK = false;
  if ( sampleSize > 0 )
  {
    // cast to double, integer could overflow
    if ( ( static_cast<double>( xSize() ) * static_cast<double>( ySize() ) / sampleSize ) > 2 ) // not perfect
    {
      QgsDebugMsg( QStringLiteral( "Approx" ) );
      bApproxOK = true;
    }
  }

  QgsDebugMsg( QStringLiteral( "xSize() = %1 ySize() = %2 sampleSize = %3 bApproxOK = %4" ).arg( xSize() ).arg( ySize() ).arg( sampleSize ).arg( bApproxOK ) );

  QgsGdalProgress myProg;
  myProg.type = QgsRaster::ProgressHistogram;
  myProg.provider = this;
  myProg.feedback = feedback;

#if 0 // this is the old method

  double myerval = ( bandStats.maximumValue - bandStats.minimumValue ) / binCount;
  GDALGetRasterHistogram( myGdalBand, bandStats.minimumValue - 0.1 * myerval,
                          bandStats.maximumValue + 0.1 * myerval, binCount, myHistogramArray,
                          ignoreOutOfRangeFlag, histogramEstimatedFlag, progressCallback,
                          &myProg ); //this is the arg for our custom gdal progress callback

#else // this is the new method, which gets a "Default" histogram

  // calculate min/max like in GDALRasterBand::GetDefaultHistogram, but don't call it directly
  // because there is no bApproxOK argument - that is lacking from the API

  // Min/max, if not specified, are set by histogramDefaults, it does not
  // set however min/max shifted to avoid rounding errors

  double myMinVal = myHistogram.minimum;
  double myMaxVal = myHistogram.maximum;

  // unapply scale anf offset for min and max
  double myScale = bandScale( bandNo );
  double myOffset = bandOffset( bandNo );
  if ( myScale != 1.0 || myOffset != 0. )
  {
    myMinVal = ( myHistogram.minimum - myOffset ) / myScale;
    myMaxVal = ( myHistogram.maximum - myOffset ) / myScale;
  }

  double dfHalfBucket = ( myMaxVal - myMinVal ) / ( 2 * myHistogram.binCount );
  myMinVal -= dfHalfBucket;
  myMaxVal += dfHalfBucket;

#if 0
  const char *pszPixelType = GDALGetMetadataItem( myGdalBand, "PIXELTYPE", "IMAGE_STRUCTURE" );
  int bSignedByte = ( pszPixelType && EQUAL( pszPixelType, "SIGNEDBYTE" ) );

  if ( GDALGetRasterDataType( myGdalBand ) == GDT_Byte && !bSignedByte )
  {
    myMinVal = -0.5;
    myMaxVal = 255.5;
  }
  else
  {
    CPLErr eErr = CE_Failure;
    double dfHalfBucket = 0;
    eErr = GDALGetRasterStatistics( myGdalBand, true, true, &myMinVal, &myMaxVal, nullptr, nullptr );
    if ( eErr != CE_None )
    {
      delete [] myHistogramArray;
      return;
    }
    dfHalfBucket = ( myMaxVal - myMinVal ) / ( 2 * binCount );
    myMinVal -= dfHalfBucket;
    myMaxVal += dfHalfBucket;
  }
#endif

  GUIntBig *myHistogramArray = new GUIntBig[myHistogram.binCount];
  CPLErr myError = GDALGetRasterHistogramEx( myGdalBand, myMinVal, myMaxVal,
                   myHistogram.binCount, myHistogramArray,
                   includeOutOfRange, bApproxOK, progressCallback,
                   &myProg ); //this is the arg for our custom gdal progress callback

  if ( myError != CE_None || ( feedback && feedback->isCanceled() ) )
  {
    QgsDebugMsg( QStringLiteral( "Cannot get histogram" ) );
    delete [] myHistogramArray;
    return myHistogram;
  }

#endif

  for ( int myBin = 0; myBin < myHistogram.binCount; myBin++ )
  {
    myHistogram.histogramVector.push_back( myHistogramArray[myBin] );
    myHistogram.nonNullCount += myHistogramArray[myBin];
    // QgsDebugMsg( "Added " + QString::number( myHistogramArray[myBin] ) + " to histogram vector" );
  }

  myHistogram.valid = true;

  delete [] myHistogramArray;

  QgsDebugMsg( ">>>>> Histogram vector now contains " + QString::number( myHistogram.histogramVector.size() ) + " elements" );

  mHistograms.append( myHistogram );
  return myHistogram;
}

/*
 * This will speed up performance at the expense of hard drive space.
 * Also, write access to the file is required for creating internal pyramids,
 * and to the directory in which the files exists if external
 * pyramids (.ovr) are to be created. If no parameter is passed in
 * it will default to nearest neighbor resampling.
 *
 * \param tryInternalFlag - Try to make the pyramids internal if supported (e.g. geotiff). If not supported it will revert to creating external .ovr file anyway.
 * \return null string on success, otherwise a string specifying error
 */
QString QgsGdalProvider::buildPyramids( const QList<QgsRasterPyramid> &rasterPyramidList,
                                        const QString &resamplingMethod, QgsRaster::RasterPyramidsFormat format,
                                        const QStringList &configOptions, QgsRasterBlockFeedback *feedback )
{
  QMutexLocker locker( mpMutex );

  //TODO: Consider making rasterPyramidList modifyable by this method to indicate if the pyramid exists after build attempt
  //without requiring the user to rebuild the pyramid list to get the updated information

  //
  // Note: Make sure the raster is not opened in write mode
  // in order to force overviews to be written to a separate file.
  // Otherwise reoopen it in read/write mode to stick overviews
  // into the same file (if supported)
  //

  if ( mGdalDataset != mGdalBaseDataset )
  {
    QgsLogger::warning( QStringLiteral( "Pyramid building not currently supported for 'warped virtual dataset'." ) );
    return QStringLiteral( "ERROR_VIRTUAL" );
  }

  // check if building internally
  if ( format == QgsRaster::PyramidsInternal )
  {

    // test if the file is writable
    //QFileInfo myQFile( mDataSource );
    QFileInfo myQFile( dataSourceUri( true ) );

    if ( !myQFile.isWritable() )
    {
      return QStringLiteral( "ERROR_WRITE_ACCESS" );
    }

    // if needed close the gdal dataset and reopen it in read / write mode
    // TODO this doesn't seem to work anymore... must fix it before 2.0!!!
    // no errors are reported, but pyramids are not present in file.
    if ( GDALGetAccess( mGdalDataset ) == GA_ReadOnly )
    {
      QgsDebugMsg( QStringLiteral( "re-opening the dataset in read/write mode" ) );
      GDALClose( mGdalDataset );
      //mGdalBaseDataset = GDALOpen( QFile::encodeName( dataSourceUri() ).constData(), GA_Update );

      mGdalBaseDataset = gdalOpen( dataSourceUri( true ).toUtf8().constData(), GA_Update );

      // if the dataset couldn't be opened in read / write mode, tell the user
      if ( !mGdalBaseDataset )
      {
        mGdalBaseDataset = gdalOpen( dataSourceUri( true ).toUtf8().constData(), GA_ReadOnly );
        //Since we are not a virtual warped dataset, mGdalDataSet and mGdalBaseDataset are supposed to be the same
        mGdalDataset = mGdalBaseDataset;
        return QStringLiteral( "ERROR_WRITE_FORMAT" );
      }
    }
  }

  // are we using Erdas Imagine external overviews?
  QgsStringMap myConfigOptionsOld;
  myConfigOptionsOld[ QStringLiteral( "USE_RRD" )] = CPLGetConfigOption( "USE_RRD", "NO" );
  myConfigOptionsOld[ QStringLiteral( "TIFF_USE_OVR" )] = CPLGetConfigOption( "TIFF_USE_OVR", "NO" );
  if ( format == QgsRaster::PyramidsErdas )
    CPLSetConfigOption( "USE_RRD", "YES" );
  else
  {
    CPLSetConfigOption( "USE_RRD", "NO" );
    if ( format == QgsRaster::PyramidsGTiff )
    {
      CPLSetConfigOption( "TIFF_USE_OVR", "YES" );
    }
  }

  // add any driver-specific configuration options, save values to be restored later
  if ( format != QgsRaster::PyramidsErdas && ! configOptions.isEmpty() )
  {
    Q_FOREACH ( const QString &option, configOptions )
    {
      QStringList opt = option.split( '=' );
      if ( opt.size() == 2 )
      {
        QByteArray key = opt[0].toLocal8Bit();
        QByteArray value = opt[1].toLocal8Bit();
        // save previous value
        myConfigOptionsOld[ opt[0] ] = QString( CPLGetConfigOption( key.data(), nullptr ) );
        // set temp. value
        CPLSetConfigOption( key.data(), value.data() );
        QgsDebugMsg( QStringLiteral( "set option %1=%2" ).arg( key.data(), value.data() ) );
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "invalid pyramid option: %1" ).arg( option ) );
      }
    }
  }

  //
  // Iterate through the Raster Layer Pyramid Vector, building any pyramid
  // marked as exists in each RasterPyramid struct.
  //
  CPLErr myError; //in case anything fails

  QVector<int> myOverviewLevelsVector;
  QList<QgsRasterPyramid>::const_iterator myRasterPyramidIterator;
  for ( myRasterPyramidIterator = rasterPyramidList.begin();
        myRasterPyramidIterator != rasterPyramidList.end();
        ++myRasterPyramidIterator )
  {
#ifdef QGISDEBUG
    QgsDebugMsg( QStringLiteral( "Build pyramids:: Level %1" ).arg( myRasterPyramidIterator->level ) );
    QgsDebugMsg( QStringLiteral( "x:%1" ).arg( myRasterPyramidIterator->xDim ) );
    QgsDebugMsg( QStringLiteral( "y:%1" ).arg( myRasterPyramidIterator->yDim ) );
    QgsDebugMsg( QStringLiteral( "exists : %1" ).arg( myRasterPyramidIterator->exists ) );
#endif
    if ( myRasterPyramidIterator->build )
    {
      QgsDebugMsg( QStringLiteral( "adding overview at level %1 to list"
                                 ).arg( myRasterPyramidIterator->level ) );
      myOverviewLevelsVector.append( myRasterPyramidIterator->level );
    }
  }
  /* From : http://www.gdal.org/classGDALDataset.html#a2aa6f88b3bbc840a5696236af11dde15
   * pszResampling : one of "NEAREST", "GAUSS", "CUBIC", "CUBICSPLINE" (GDAL >= 2.0),
   * "LANCZOS" ( GDAL >= 2.0), "AVERAGE", "MODE" or "NONE" controlling the downsampling method applied.
   * nOverviews : number of overviews to build.
   * panOverviewList : the list of overview decimation factors to build.
   * nListBands : number of bands to build overviews for in panBandList. Build for all bands if this is 0.
   * panBandList : list of band numbers.
   * pfnProgress : a function to call to report progress, or nullptr.
   * pProgressData : application data to pass to the progress function.
   */

  // resampling method is now passed directly, via QgsRasterDataProvider::pyramidResamplingArg()
  // average_mp and average_magphase have been removed from the gui
  QByteArray ba = resamplingMethod.toLocal8Bit();
  const char *method = ba.data();

  //build the pyramid and show progress to console
  QgsDebugMsg( QStringLiteral( "Building overviews at %1 levels using resampling method %2"
                             ).arg( myOverviewLevelsVector.size() ).arg( method ) );
  try
  {
    //build the pyramid and show progress to console
    QgsGdalProgress myProg;
    myProg.type = QgsRaster::ProgressPyramids;
    myProg.provider = this;
    myProg.feedback = feedback;
    myError = GDALBuildOverviews( mGdalBaseDataset, method,
                                  myOverviewLevelsVector.size(), myOverviewLevelsVector.data(),
                                  0, nullptr,
                                  progressCallback, &myProg ); //this is the arg for the gdal progress callback

    if ( ( feedback && feedback->isCanceled() ) || myError == CE_Failure || CPLGetLastErrorNo() == CPLE_NotSupported )
    {
      QgsDebugMsg( QStringLiteral( "Building pyramids failed using resampling method [%1]" ).arg( method ) );
      //something bad happenend
      //QString myString = QString (CPLGetLastError());
      GDALClose( mGdalBaseDataset );
      mGdalBaseDataset = gdalOpen( dataSourceUri( true ).toUtf8().constData(), mUpdate ? GA_Update : GA_ReadOnly );
      //Since we are not a virtual warped dataset, mGdalDataSet and mGdalBaseDataset are supposed to be the same
      mGdalDataset = mGdalBaseDataset;

      // restore former configOptions
      for ( QgsStringMap::const_iterator it = myConfigOptionsOld.constBegin();
            it != myConfigOptionsOld.constEnd(); ++it )
      {
        QByteArray key = it.key().toLocal8Bit();
        QByteArray value = it.value().toLocal8Bit();
        CPLSetConfigOption( key.data(), value.data() );
      }

      // TODO print exact error message
      if ( feedback && feedback->isCanceled() )
        return QStringLiteral( "CANCELED" );

      return QStringLiteral( "FAILED_NOT_SUPPORTED" );
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "Building pyramids finished OK" ) );
      //make sure the raster knows it has pyramids
      mHasPyramids = true;
    }
  }
  catch ( CPLErr )
  {
    QgsLogger::warning( QStringLiteral( "Pyramid overview building failed!" ) );
  }

  // restore former configOptions
  for ( QgsStringMap::const_iterator it = myConfigOptionsOld.constBegin();
        it != myConfigOptionsOld.constEnd(); ++it )
  {
    QByteArray key = it.key().toLocal8Bit();
    QByteArray value = it.value().toLocal8Bit();
    CPLSetConfigOption( key.data(), value.data() );
  }

  QgsDebugMsg( QStringLiteral( "Pyramid overviews built" ) );

  // Observed problem: if a *.rrd file exists and GDALBuildOverviews() is called,
  // the *.rrd is deleted and no overviews are created, if GDALBuildOverviews()
  // is called next time, it crashes somewhere in GDAL:
  // https://trac.osgeo.org/gdal/ticket/4831
  // Crash can be avoided if dataset is reopened, fixed in GDAL 1.9.2
  if ( format == QgsRaster::PyramidsInternal )
  {
    QgsDebugMsg( QStringLiteral( "Reopening dataset ..." ) );
    //close the gdal dataset and reopen it in read only mode
    GDALClose( mGdalBaseDataset );
    mGdalBaseDataset = gdalOpen( dataSourceUri( true ).toUtf8().constData(), mUpdate ? GA_Update : GA_ReadOnly );
    //Since we are not a virtual warped dataset, mGdalDataSet and mGdalBaseDataset are supposed to be the same
    mGdalDataset = mGdalBaseDataset;
  }

  //emit drawingProgress( 0, 0 );
  return QString(); // returning null on success
}

#if 0
QList<QgsRasterPyramid> QgsGdalProvider::buildPyramidList()
{
  //
  // First we build up a list of potential pyramid layers
  //
  int myWidth = mWidth;
  int myHeight = mHeight;
  int myDivisor = 2;

  GDALRasterBandH myGDALBand = GDALGetRasterBand( mGdalDataset, 1 ); //just use the first band

  mPyramidList.clear();
  QgsDebugMsg( QStringLiteral( "Building initial pyramid list" ) );
  while ( ( myWidth / myDivisor > 32 ) && ( ( myHeight / myDivisor ) > 32 ) )
  {

    QgsRasterPyramid myRasterPyramid;
    myRasterPyramid.level = myDivisor;
    myRasterPyramid.xDim = ( int )( 0.5 + ( myWidth / static_cast<double>( myDivisor ) ) );
    myRasterPyramid.yDim = ( int )( 0.5 + ( myHeight / static_cast<double>( myDivisor ) ) );
    myRasterPyramid.exists = false;

    QgsDebugMsg( QStringLiteral( "Pyramid %1 xDim %2 yDim %3" ).arg( myRasterPyramid.level ).arg( myRasterPyramid.xDim ).arg( myRasterPyramid.yDim ) );

    //
    // Now we check if it actually exists in the raster layer
    // and also adjust the dimensions if the dimensions calculated
    // above are only a near match.
    //
    const int myNearMatchLimit = 5;
    if ( GDALGetOverviewCount( myGDALBand ) > 0 )
    {
      int myOverviewCount;
      for ( myOverviewCount = 0;
            myOverviewCount < GDALGetOverviewCount( myGDALBand );
            ++myOverviewCount )
      {
        GDALRasterBandH myOverview;
        myOverview = GDALGetOverview( myGDALBand, myOverviewCount );
        int myOverviewXDim = GDALGetRasterBandXSize( myOverview );
        int myOverviewYDim = GDALGetRasterBandYSize( myOverview );
        //
        // here is where we check if its a near match:
        // we will see if its within 5 cells either side of
        //
        QgsDebugMsg( "Checking whether " + QString::number( myRasterPyramid.xDim ) + " x " +
                     QString::number( myRasterPyramid.yDim ) + " matches " +
                     QString::number( myOverviewXDim ) + " x " + QString::number( myOverviewYDim ) );


        if ( ( myOverviewXDim <= ( myRasterPyramid.xDim + myNearMatchLimit ) ) &&
             ( myOverviewXDim >= ( myRasterPyramid.xDim - myNearMatchLimit ) ) &&
             ( myOverviewYDim <= ( myRasterPyramid.yDim + myNearMatchLimit ) ) &&
             ( myOverviewYDim >= ( myRasterPyramid.yDim - myNearMatchLimit ) ) )
        {
          //right we have a match so adjust the a / y before they get added to the list
          myRasterPyramid.xDim = myOverviewXDim;
          myRasterPyramid.yDim = myOverviewYDim;
          myRasterPyramid.exists = true;
          QgsDebugMsg( QStringLiteral( ".....YES!" ) );
        }
        else
        {
          //no match
          QgsDebugMsg( QStringLiteral( ".....no." ) );
        }
      }
    }
    mPyramidList.append( myRasterPyramid );
    //sqare the divisor each step
    myDivisor = ( myDivisor * 2 );
  }

  return mPyramidList;
}
#endif

QList<QgsRasterPyramid> QgsGdalProvider::buildPyramidList( QList<int> overviewList )
{
  QMutexLocker locker( mpMutex );

  int myWidth = mWidth;
  int myHeight = mHeight;
  GDALRasterBandH myGDALBand = GDALGetRasterBand( mGdalDataset, 1 ); //just use the first band

  mPyramidList.clear();

  // if overviewList is empty (default) build the pyramid list
  if ( overviewList.isEmpty() )
  {
    int myDivisor = 2;

    QgsDebugMsg( QStringLiteral( "Building initial pyramid list" ) );

    while ( ( myWidth / myDivisor > 32 ) && ( ( myHeight / myDivisor ) > 32 ) )
    {
      overviewList.append( myDivisor );
      //sqare the divisor each step
      myDivisor = ( myDivisor * 2 );
    }
  }

  // loop over pyramid list
  Q_FOREACH ( int myDivisor, overviewList )
  {
    //
    // First we build up a list of potential pyramid layers
    //

    QgsRasterPyramid myRasterPyramid;
    myRasterPyramid.level = myDivisor;
    myRasterPyramid.xDim = ( int )( 0.5 + ( myWidth / static_cast<double>( myDivisor ) ) ); // NOLINT
    myRasterPyramid.yDim = ( int )( 0.5 + ( myHeight / static_cast<double>( myDivisor ) ) ); // NOLINT
    myRasterPyramid.exists = false;

    QgsDebugMsg( QStringLiteral( "Pyramid %1 xDim %2 yDim %3" ).arg( myRasterPyramid.level ).arg( myRasterPyramid.xDim ).arg( myRasterPyramid.yDim ) );

    //
    // Now we check if it actually exists in the raster layer
    // and also adjust the dimensions if the dimensions calculated
    // above are only a near match.
    //
    const int myNearMatchLimit = 5;
    if ( GDALGetOverviewCount( myGDALBand ) > 0 )
    {
      int myOverviewCount;
      for ( myOverviewCount = 0;
            myOverviewCount < GDALGetOverviewCount( myGDALBand );
            ++myOverviewCount )
      {
        GDALRasterBandH myOverview;
        myOverview = GDALGetOverview( myGDALBand, myOverviewCount );
        int myOverviewXDim = GDALGetRasterBandXSize( myOverview );
        int myOverviewYDim = GDALGetRasterBandYSize( myOverview );
        //
        // here is where we check if its a near match:
        // we will see if its within 5 cells either side of
        //
        QgsDebugMsg( "Checking whether " + QString::number( myRasterPyramid.xDim ) + " x " +
                     QString::number( myRasterPyramid.yDim ) + " matches " +
                     QString::number( myOverviewXDim ) + " x " + QString::number( myOverviewYDim ) );


        if ( ( myOverviewXDim <= ( myRasterPyramid.xDim + myNearMatchLimit ) ) &&
             ( myOverviewXDim >= ( myRasterPyramid.xDim - myNearMatchLimit ) ) &&
             ( myOverviewYDim <= ( myRasterPyramid.yDim + myNearMatchLimit ) ) &&
             ( myOverviewYDim >= ( myRasterPyramid.yDim - myNearMatchLimit ) ) )
        {
          //right we have a match so adjust the a / y before they get added to the list
          myRasterPyramid.xDim = myOverviewXDim;
          myRasterPyramid.yDim = myOverviewYDim;
          myRasterPyramid.exists = true;
          QgsDebugMsg( QStringLiteral( ".....YES!" ) );
        }
        else
        {
          //no match
          QgsDebugMsg( QStringLiteral( ".....no." ) );
        }
      }
    }
    mPyramidList.append( myRasterPyramid );
  }

  return mPyramidList;
}

QStringList QgsGdalProvider::subLayers() const
{
  return mSubLayers;
}

QGISEXTERN int dataCapabilities()
{
  return QgsDataProvider::File | QgsDataProvider::Dir | QgsDataProvider::Net;
}

QGISEXTERN QVariantMap decodeUri( const QString &uri )
{
  QString path = uri;
  QString layerName;

  QString vsiPrefix = qgsVsiPrefix( path );
  if ( !path.isEmpty() )
    path = path.mid( vsiPrefix.count() );

  if ( path.indexOf( ':' ) != -1 )
  {
    QStringList parts = path.split( ':' );
    if ( parts[0].toLower() == QStringLiteral( "gpkg" ) )
    {
      parts.removeFirst();
      // Handle windows paths - which has an extra colon - and unix paths
      if ( ( parts[0].length() > 1 && parts.count() > 1 ) || parts.count() > 2 )
      {
        layerName = parts[parts.length() - 1];
        parts.removeLast();
      }
      path  = parts.join( ':' );
    }
  }

  QVariantMap uriComponents;
  uriComponents.insert( QStringLiteral( "path" ), path );
  uriComponents.insert( QStringLiteral( "layerName" ), layerName );
  return uriComponents;
}

/**
 * Class factory to return a pointer to a newly created
 * QgsGdalProvider object
 */
QGISEXTERN QgsGdalProvider *classFactory( const QString *uri, const QgsDataProvider::ProviderOptions &options )
{
  return new QgsGdalProvider( *uri, options );
}

/**
 * Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey()
{
  return PROVIDER_KEY;
}

/**
 * Required description function
 */
QGISEXTERN QString description()
{
  return PROVIDER_DESCRIPTION;
}

/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider()
{
  return true;
}

/**

  Convenience function for readily creating file filters.

  Given a long name for a file filter and a regular expression, return
  a file filter string suitable for use in a QFileDialog::OpenFiles()
  call.  The regular express, glob, will have both all lower and upper
  case versions added.

  \note

  Copied from qgisapp.cpp.

  \todo XXX This should probably be generalized and moved to a standard
            utility type thingy.

*/
static QString createFileFilter_( QString const &longName, QString const &glob )
{
  // return longName + " [GDAL] (" + glob.toLower() + ' ' + glob.toUpper() + ");;";
  return longName + " (" + glob.toLower() + ' ' + glob.toUpper() + ");;";
} // createFileFilter_

void buildSupportedRasterFileFilterAndExtensions( QString &fileFiltersString, QStringList &extensions, QStringList &wildcards )
{

  // then iterate through all of the supported drivers, adding the
  // corresponding file filter

  GDALDriverH myGdalDriver;           // current driver

  QStringList catchallFilter;   // for Any file(*.*), but also for those
  // drivers with no specific file filter

  GDALDriverH jp2Driver = nullptr; // first JPEG2000 driver found

  QgsGdalProviderBase::registerGdalDrivers();

  // Grind through all the drivers and their respective metadata.
  // We'll add a file filter for those drivers that have a file
  // extension defined for them; the others, well, even though
  // theoreticaly we can open those files because there exists a
  // driver for them, the user will have to use the "All Files" to
  // open datasets with no explicitly defined file name extension.

  fileFiltersString.clear();

  QgsDebugMsg( QStringLiteral( "GDAL driver count: %1" ).arg( GDALGetDriverCount() ) );

  for ( int i = 0; i < GDALGetDriverCount(); ++i )
  {
    myGdalDriver = GDALGetDriver( i );

    Q_CHECK_PTR( myGdalDriver ); // NOLINT

    if ( !myGdalDriver )
    {
      QgsLogger::warning( "unable to get driver " + QString::number( i ) );
      continue;
    }

    // in GDAL 2.0 vector and mixed drivers are returned by GDALGetDriver, so filter out non-raster drivers
    if ( QString( GDALGetMetadataItem( myGdalDriver, GDAL_DCAP_RASTER, nullptr ) ) != QLatin1String( "YES" ) )
      continue;

    // now we need to see if the driver is for something currently
    // supported; if not, we give it a miss for the next driver

    QString myGdalDriverDescription = GDALGetDescription( myGdalDriver );
    if ( myGdalDriverDescription == QLatin1String( "BIGGIF" ) )
    {
      // BIGGIF is a technical driver. The plain GIF driver will do
      continue;
    }

    // QgsDebugMsg(QString("got driver string %1").arg(myGdalDriverDescription));

    QString myGdalDriverExtensions = GDALGetMetadataItem( myGdalDriver, GDAL_DMD_EXTENSIONS, "" );
    QString myGdalDriverLongName = GDALGetMetadataItem( myGdalDriver, GDAL_DMD_LONGNAME, "" );
    // remove any superfluous (.*) strings at the end as
    // they'll confuse QFileDialog::getOpenFileNames()
    myGdalDriverLongName.remove( QRegExp( "\\(.*\\)$" ) );

    // if we have both the file name extension and the long name,
    // then we've all the information we need for the current
    // driver; therefore emit a file filter string and move to
    // the next driver
    if ( !( myGdalDriverExtensions.isEmpty() || myGdalDriverLongName.isEmpty() ) )
    {
      const QStringList splitExtensions = myGdalDriverExtensions.split( ' ', QString::SkipEmptyParts );

      // XXX add check for SDTS; in that case we want (*CATD.DDF)
      QString glob;

      for ( const QString &ext : splitExtensions )
      {
        // This hacking around that removes '/' is no longer necessary with GDAL 2.3
        extensions << QString( ext ).remove( '/' ).remove( '*' ).remove( '.' );
        if ( !glob.isEmpty() )
          glob += QLatin1String( " " );
        glob += "*." + QString( ext ).replace( '/', QLatin1String( " *." ) );
      }

      // Add only the first JP2 driver found to the filter list (it's the one GDAL uses)
      if ( myGdalDriverDescription == QLatin1String( "JPEG2000" ) ||
           myGdalDriverDescription.startsWith( QLatin1String( "JP2" ) ) ) // JP2ECW, JP2KAK, JP2MrSID
      {
        if ( jp2Driver )
          continue; // skip if already found a JP2 driver

        jp2Driver = myGdalDriver;   // first JP2 driver found
        if ( !glob.contains( "j2k" ) )
        {
          glob += QLatin1String( " *.j2k" );         // add alternate extension
          extensions << QStringLiteral( "j2k" );
        }
      }
      else if ( myGdalDriverDescription == QLatin1String( "VRT" ) )
      {
        glob += QLatin1String( " *.ovr" );
        extensions << QStringLiteral( "ovr" );
      }

      fileFiltersString += createFileFilter_( myGdalDriverLongName, glob );
    }


    //QgsDebugMsg(QString("got driver Desc=%1 LongName=%2").arg(myGdalDriverDescription).arg(myGdalDriverLongName));

    if ( myGdalDriverExtensions.isEmpty() && !myGdalDriverLongName.isEmpty() )
    {
      // Then what we have here is a driver with no corresponding
      // file extension; e.g., GRASS.  In which case we append the
      // string to the "catch-all" which will match all file types.
      // (I.e., "*.*") We use the driver description instead of the
      // long time to prevent the catch-all line from getting too
      // large.

      // ... OTOH, there are some drivers with missing
      // DMD_EXTENSION; so let's check for them here and handle
      // them appropriately

      if ( myGdalDriverDescription.startsWith( QLatin1String( "AIG" ) ) )
      {
        fileFiltersString += createFileFilter_( myGdalDriverLongName, QStringLiteral( "hdr.adf" ) );
        wildcards << QStringLiteral( "hdr.adf" );
      }
#if !(GDAL_VERSION_MAJOR > 2 || (GDAL_VERSION_MAJOR == 2 && GDAL_VERSION_MINOR >= 3))
      else if ( myGdalDriverDescription.startsWith( QLatin1String( "EHdr" ) ) )
      {
        // Fixed in GDAL 2.3
        fileFiltersString += createFileFilter_( myGdalDriverLongName, QStringLiteral( "*.bil" ) );
        extensions << QStringLiteral( "bil" );
      }
      else if ( myGdalDriverDescription == QLatin1String( "ERS" ) )
      {
        // Fixed in GDAL 2.3
        fileFiltersString += createFileFilter_( myGdalDriverLongName, QStringLiteral( "*.ers" ) );
        extensions << QStringLiteral( "ers" );
      }
#endif
      else
      {
        catchallFilter << QString( GDALGetDescription( myGdalDriver ) );
      }
    } // each loaded GDAL driver

  }                           // each loaded GDAL driver

  // sort file filters alphabetically
  QStringList filters = fileFiltersString.split( QStringLiteral( ";;" ), QString::SkipEmptyParts );
  filters.sort();
  fileFiltersString = filters.join( QStringLiteral( ";;" ) ) + ";;";

  // VSIFileHandler (see qgsogrprovider.cpp) - second
  QgsSettings settings;
  if ( settings.value( QStringLiteral( "qgis/scanZipInBrowser2" ), "basic" ).toString() != QLatin1String( "no" ) )
  {
    fileFiltersString.prepend( createFileFilter_( QObject::tr( "GDAL/OGR VSIFileHandler" ), QStringLiteral( "*.zip *.gz *.tar *.tar.gz *.tgz" ) ) );
    extensions << QStringLiteral( "zip" ) << QStringLiteral( "gz" ) << QStringLiteral( "tar" ) << QStringLiteral( "tar.gz" ) << QStringLiteral( "tgz" );
  }

  // can't forget the default case - first
  fileFiltersString.prepend( QObject::tr( "All files" ) + " (*);;" );

  // cleanup
  if ( fileFiltersString.endsWith( QLatin1String( ";;" ) ) ) fileFiltersString.chop( 2 );

  QgsDebugMsg( "Raster filter list built: " + fileFiltersString );
  QgsDebugMsg( "Raster extension list built: " + extensions.join( " " ) );
}                               // buildSupportedRasterFileFilter_()

QGISEXTERN bool isValidRasterFileName( QString const &fileNameQString, QString &retErrMsg )
{
  gdal::dataset_unique_ptr myDataset;

  QgsGdalProviderBase::registerGdalDrivers();

  CPLErrorReset();

  QString fileName = fileNameQString;

  // Try to open using VSIFileHandler (see qgsogrprovider.cpp)
  // TODO suppress error messages and report in debug, like in OGR provider
  QString vsiPrefix = QgsZipItem::vsiPrefix( fileName );
  if ( !vsiPrefix.isEmpty() )
  {
    if ( !fileName.startsWith( vsiPrefix ) )
      fileName = vsiPrefix + fileName;
    QgsDebugMsg( QStringLiteral( "Trying %1 syntax, fileName= %2" ).arg( vsiPrefix, fileName ) );
  }

  //open the file using gdal making sure we have handled locale properly
  //myDataset = GDALOpen( QFile::encodeName( fileNameQString ).constData(), GA_ReadOnly );
  myDataset.reset( QgsGdalProviderBase::gdalOpen( fileName.toUtf8().constData(), GA_ReadOnly ) );
  if ( !myDataset )
  {
    if ( CPLGetLastErrorNo() != CPLE_OpenFailed )
      retErrMsg = QString::fromUtf8( CPLGetLastErrorMsg() );
    return false;
  }
  else if ( GDALGetRasterCount( myDataset.get() ) == 0 )
  {
    QStringList layers = QgsGdalProvider::subLayers( myDataset.get() );
    if ( layers.isEmpty() )
    {
      retErrMsg = QObject::tr( "This raster file has no bands and is invalid as a raster layer." );
      return false;
    }
    return true;
  }
  else
  {
    return true;
  }
}

bool QgsGdalProvider::hasStatistics( int bandNo,
                                     int stats,
                                     const QgsRectangle &boundingBox,
                                     int sampleSize )
{
  QMutexLocker locker( mpMutex );
  if ( !initIfNeeded() )
    return false;

  QgsDebugMsg( QStringLiteral( "theBandNo = %1 sampleSize = %2" ).arg( bandNo ).arg( sampleSize ) );

  // First check if cached in mStatistics
  if ( QgsRasterDataProvider::hasStatistics( bandNo, stats, boundingBox, sampleSize ) )
  {
    return true;
  }

  QgsRasterBandStats myRasterBandStats;
  initStatistics( myRasterBandStats, bandNo, stats, boundingBox, sampleSize );

  if ( ( sourceHasNoDataValue( bandNo ) && !useSourceNoDataValue( bandNo ) ) ||
       !userNoDataValues( bandNo ).isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "Custom no data values -> GDAL statistics not sufficient." ) );
    return false;
  }

  // If not cached, check if supported by GDAL
  int supportedStats = QgsRasterBandStats::Min | QgsRasterBandStats::Max
                       | QgsRasterBandStats::Range | QgsRasterBandStats::Mean
                       | QgsRasterBandStats::StdDev;

  if ( myRasterBandStats.extent != extent() ||
       ( stats & ( ~supportedStats ) ) )
  {
    QgsDebugMsg( QStringLiteral( "Not supported by GDAL." ) );
    return false;
  }

  QgsDebugMsg( QStringLiteral( "Looking for GDAL statistics" ) );

  GDALRasterBandH myGdalBand = getBand( bandNo );
  if ( ! myGdalBand )
  {
    return false;
  }

  int bApproxOK = false;
  if ( sampleSize > 0 )
  {
    if ( ( static_cast<double>( xSize() ) * static_cast<double>( ySize() ) / sampleSize ) > 2 ) // not perfect
    {
      bApproxOK = true;
    }
  }

  // Params in GDALGetRasterStatistics must not be nullptr otherwise GDAL returns
  // without error even if stats are not cached
  double dfMin, dfMax, dfMean, dfStdDev;
  double *pdfMin = &dfMin;
  double *pdfMax = &dfMax;
  double *pdfMean = &dfMean;
  double *pdfStdDev = &dfStdDev;

  if ( !( stats & QgsRasterBandStats::Min ) ) pdfMin = nullptr;
  if ( !( stats & QgsRasterBandStats::Max ) ) pdfMax = nullptr;
  if ( !( stats & QgsRasterBandStats::Mean ) ) pdfMean = nullptr;
  if ( !( stats & QgsRasterBandStats::StdDev ) ) pdfStdDev = nullptr;

  // try to fetch the cached stats (bForce=FALSE)
  // Unfortunately GDALGetRasterStatistics() does not work as expected according to
  // API doc, if bApproxOK=false and bForce=false/true and exact statistics
  // (from all raster pixels) are not available/cached, it should return CE_Warning.
  // Instead, it is giving estimated (from sample) cached statistics and it returns CE_None.
  // see above and https://trac.osgeo.org/gdal/ticket/4857
  // -> Cannot used cached GDAL stats for exact
  if ( !bApproxOK ) return false;

  CPLErr myerval = GDALGetRasterStatistics( myGdalBand, bApproxOK, true, pdfMin, pdfMax, pdfMean, pdfStdDev );

  if ( CE_None == myerval ) // CE_Warning if cached not found
  {
    QgsDebugMsg( QStringLiteral( "GDAL has cached statistics" ) );
    return true;
  }

  return false;
}

QgsRasterBandStats QgsGdalProvider::bandStatistics( int bandNo, int stats, const QgsRectangle &boundingBox, int sampleSize, QgsRasterBlockFeedback *feedback )
{
  QMutexLocker locker( mpMutex );
  if ( !initIfNeeded() )
    return QgsRasterBandStats();

  QgsDebugMsg( QStringLiteral( "theBandNo = %1 sampleSize = %2" ).arg( bandNo ).arg( sampleSize ) );

  // TODO: null values set on raster layer!!!

  // Currently there is no API in GDAL to collect statistics of specified extent
  // or with defined sample size. We check first if we have cached stats, if not,
  // and it is not possible to use GDAL we call generic provider method,
  // otherwise we use GDAL (faster, cache)

  QgsRasterBandStats myRasterBandStats;
  initStatistics( myRasterBandStats, bandNo, stats, boundingBox, sampleSize );

  Q_FOREACH ( const QgsRasterBandStats &stats, mStatistics )
  {
    if ( stats.contains( myRasterBandStats ) )
    {
      QgsDebugMsg( QStringLiteral( "Using cached statistics." ) );
      return stats;
    }
  }

  // We cannot use GDAL stats if user disabled src no data value or set
  // custom  no data values
  if ( ( sourceHasNoDataValue( bandNo ) && !useSourceNoDataValue( bandNo ) ) ||
       !userNoDataValues( bandNo ).isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "Custom no data values, using generic statistics." ) );
    return QgsRasterDataProvider::bandStatistics( bandNo, stats, boundingBox, sampleSize, feedback );
  }

  int supportedStats = QgsRasterBandStats::Min | QgsRasterBandStats::Max
                       | QgsRasterBandStats::Range | QgsRasterBandStats::Mean
                       | QgsRasterBandStats::StdDev;

  QgsDebugMsg( QStringLiteral( "theStats = %1 supportedStats = %2" ).arg( stats, 0, 2 ).arg( supportedStats, 0, 2 ) );

  if ( myRasterBandStats.extent != extent() ||
       ( stats & ( ~supportedStats ) ) )
  {
    QgsDebugMsg( QStringLiteral( "Statistics not supported by provider, using generic statistics." ) );
    return QgsRasterDataProvider::bandStatistics( bandNo, stats, boundingBox, sampleSize, feedback );
  }

  QgsDebugMsg( QStringLiteral( "Using GDAL statistics." ) );
  GDALRasterBandH myGdalBand = getBand( bandNo );

  //int bApproxOK = false; //as we asked for stats, don't get approx values
  // GDAL does not have sample size parameter in API, just bApproxOK or not,
  // we decide if approximation should be used according to
  // total size / sample size ration
  int bApproxOK = false;
  if ( sampleSize > 0 )
  {
    if ( ( static_cast<double>( xSize() ) * static_cast<double>( ySize() ) / sampleSize ) > 2 ) // not perfect
    {
      bApproxOK = true;
    }
  }

  QgsDebugMsg( QStringLiteral( "bApproxOK = %1" ).arg( bApproxOK ) );

  double pdfMin;
  double pdfMax;
  double pdfMean;
  double pdfStdDev;
  QgsGdalProgress myProg;
  myProg.type = QgsRaster::ProgressHistogram;
  myProg.provider = this;
  myProg.feedback = feedback;

  // try to fetch the cached stats (bForce=FALSE)
  // GDALGetRasterStatistics() do not work correctly with bApproxOK=false and bForce=false/true
  // see above and https://trac.osgeo.org/gdal/ticket/4857
  // -> Cannot used cached GDAL stats for exact

  CPLErr myerval =
    GDALGetRasterStatistics( myGdalBand, bApproxOK, true, &pdfMin, &pdfMax, &pdfMean, &pdfStdDev );

  QgsDebugMsg( QStringLiteral( "myerval = %1" ).arg( myerval ) );

  // if cached stats are not found, compute them
  if ( !bApproxOK || CE_None != myerval )
  {
    QgsDebugMsg( QStringLiteral( "Calculating statistics by GDAL" ) );
    myerval = GDALComputeRasterStatistics( myGdalBand, bApproxOK,
                                           &pdfMin, &pdfMax, &pdfMean, &pdfStdDev,
                                           progressCallback, &myProg );
    mStatisticsAreReliable = true;
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "Using GDAL cached statistics" ) );
  }

  if ( feedback && feedback->isCanceled() )
    return myRasterBandStats;

  // if stats are found populate the QgsRasterBandStats object
  if ( CE_None == myerval )
  {
    myRasterBandStats.bandNumber = bandNo;
    myRasterBandStats.range = pdfMax - pdfMin;
    myRasterBandStats.minimumValue = pdfMin;
    myRasterBandStats.maximumValue = pdfMax;
    //calculate the mean
    myRasterBandStats.mean = pdfMean;
    myRasterBandStats.sum = 0; //not available via gdal
    //myRasterBandStats.elementCount = mWidth * mHeight;
    // Sum of non NULL
    myRasterBandStats.elementCount = 0; //not available via gdal
    myRasterBandStats.sumOfSquares = 0; //not available via gdal
    myRasterBandStats.stdDev = pdfStdDev;
    myRasterBandStats.statsGathered = QgsRasterBandStats::Min | QgsRasterBandStats::Max
                                      | QgsRasterBandStats::Range | QgsRasterBandStats::Mean
                                      | QgsRasterBandStats::StdDev;

    // define if the band has scale and offset to apply
    double myScale = bandScale( bandNo );
    double myOffset = bandOffset( bandNo );
    if ( myScale != 1.0 || myOffset != 0.0 )
    {
      if ( myScale < 0.0 )
      {
        // update Min and Max value
        myRasterBandStats.minimumValue = pdfMax * myScale + myOffset;
        myRasterBandStats.maximumValue = pdfMin * myScale + myOffset;
        // update the range
        myRasterBandStats.range = ( pdfMin - pdfMax ) * myScale;
        // update standard deviation
        myRasterBandStats.stdDev = -1.0 * pdfStdDev * myScale;
      }
      else
      {
        // update Min and Max value
        myRasterBandStats.minimumValue = pdfMin * myScale + myOffset;
        myRasterBandStats.maximumValue = pdfMax * myScale + myOffset;
        // update the range
        myRasterBandStats.range = ( pdfMax - pdfMin ) * myScale;
        // update standard deviation
        myRasterBandStats.stdDev = pdfStdDev * myScale;
      }
      // update the mean
      myRasterBandStats.mean = pdfMean * myScale + myOffset;
    }

#ifdef QGISDEBUG
    QgsDebugMsg( QStringLiteral( "************ STATS **************" ) );
    QgsDebugMsg( QStringLiteral( "MIN %1" ).arg( myRasterBandStats.minimumValue ) );
    QgsDebugMsg( QStringLiteral( "MAX %1" ).arg( myRasterBandStats.maximumValue ) );
    QgsDebugMsg( QStringLiteral( "RANGE %1" ).arg( myRasterBandStats.range ) );
    QgsDebugMsg( QStringLiteral( "MEAN %1" ).arg( myRasterBandStats.mean ) );
    QgsDebugMsg( QStringLiteral( "STDDEV %1" ).arg( myRasterBandStats.stdDev ) );
#endif
  }

  mStatistics.append( myRasterBandStats );
  return myRasterBandStats;

} // QgsGdalProvider::bandStatistics

bool QgsGdalProvider::initIfNeeded()
{
  if ( mHasInit )
    return mValid;

  mHasInit = true;

  QString gdalUri = dataSourceUri( true );

  // Try to open using VSIFileHandler (see qgsogrprovider.cpp)
  QString vsiPrefix = QgsZipItem::vsiPrefix( gdalUri );
  if ( !vsiPrefix.isEmpty() )
  {
    if ( !gdalUri.startsWith( vsiPrefix ) )
      setDataSourceUri( vsiPrefix + gdalUri );
    QgsDebugMsg( QStringLiteral( "Trying %1 syntax, uri= %2" ).arg( vsiPrefix, dataSourceUri() ) );
  }

  gdalUri = dataSourceUri( true );

  CPLErrorReset();
  mGdalBaseDataset = gdalOpen( gdalUri.toUtf8().constData(), mUpdate ? GA_Update : GA_ReadOnly );

  if ( !mGdalBaseDataset )
  {
    QString msg = QStringLiteral( "Cannot open GDAL dataset %1:\n%2" ).arg( dataSourceUri(), QString::fromUtf8( CPLGetLastErrorMsg() ) );
    appendError( ERRMSG( msg ) );
    return false;
  }

  QgsDebugMsg( QStringLiteral( "GdalDataset opened" ) );

  initBaseDataset();
  return mValid;
}


void QgsGdalProvider::initBaseDataset()
{
  mHasInit = true;
  mValid = true;
#if 0
  for ( int i = 0; i < GDALGetRasterCount( mGdalBaseDataset ); i++ )
  {
    mMinMaxComputed.append( false );
    mMinimum.append( 0 );
    mMaximum.append( 0 );
  }
#endif
  // Check if we need a warped VRT for this file.
  bool hasGeoTransform = GDALGetGeoTransform( mGdalBaseDataset, mGeoTransform ) == CE_None;
  if ( ( hasGeoTransform
         && ( mGeoTransform[1] < 0.0
              || mGeoTransform[2] != 0.0
              || mGeoTransform[4] != 0.0
              || mGeoTransform[5] > 0.0 ) )
       || GDALGetGCPCount( mGdalBaseDataset ) > 0
       || GDALGetMetadata( mGdalBaseDataset, "RPC" ) )
  {
    QgsLogger::warning( QStringLiteral( "Creating Warped VRT." ) );

    mGdalDataset =
      GDALAutoCreateWarpedVRT( mGdalBaseDataset, nullptr, nullptr,
                               GRA_NearestNeighbour, 0.2, nullptr );

    if ( !mGdalDataset )
    {
      QgsLogger::warning( QStringLiteral( "Warped VRT Creation failed." ) );
      mGdalDataset = mGdalBaseDataset;
    }
    else
    {
      hasGeoTransform = GDALGetGeoTransform( mGdalDataset, mGeoTransform ) == CE_None;
    }
  }
  else
  {
    mGdalDataset = mGdalBaseDataset;
  }

  if ( !hasGeoTransform )
  {
    // Initialize the affine transform matrix
    mGeoTransform[0] = 0;
    mGeoTransform[1] = 1;
    mGeoTransform[2] = 0;
    mGeoTransform[3] = 0;
    mGeoTransform[4] = 0;
    mGeoTransform[5] = -1;
  }

  // get sublayers
  mSubLayers = QgsGdalProvider::subLayers( mGdalDataset );

  // check if this file has bands or subdatasets
  CPLErrorReset();
  GDALRasterBandH myGDALBand = GDALGetRasterBand( mGdalDataset, 1 ); //just use the first band
  if ( !myGDALBand )
  {
    QString msg = QString::fromUtf8( CPLGetLastErrorMsg() );

    // if there are no subdatasets, then close the dataset
    if ( mSubLayers.isEmpty() )
    {
      appendError( ERRMSG( tr( "Cannot get GDAL raster band: %1" ).arg( msg ) ) );

      closeDataset();
      return;
    }
    // if there are subdatasets, leave the dataset open for subsequent queries
    else
    {
      QgsDebugMsg( QObject::tr( "Cannot get GDAL raster band: %1" ).arg( msg ) +
                   QString( " but dataset has %1 subdatasets" ).arg( mSubLayers.size() ) );
      mValid = false;
      return;
    }
  }

  // check if this file has pyramids
  mHasPyramids = gdalGetOverviewCount( myGDALBand ) > 0;

  // Get the layer's projection info and set up the
  // QgsCoordinateTransform for this layer
  // NOTE: we must do this before metadata is called

  if ( !crsFromWkt( GDALGetProjectionRef( mGdalDataset ) ) &&
       !crsFromWkt( GDALGetGCPProjection( mGdalDataset ) ) )
  {
    if ( mGdalBaseDataset != mGdalDataset &&
         GDALGetMetadata( mGdalBaseDataset, "RPC" ) )
    {
      // Warped VRT of RPC is in EPSG:4326
      mCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( QStringLiteral( "EPSG:4326" ) );
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "No valid CRS identified" ) );
    }
  }

  //set up the coordinat transform - in the case of raster this is mainly used to convert
  //the inverese projection of the map extents of the canvas when zooming in etc. so
  //that they match the coordinate system of this layer
  //QgsDebugMsg( "Layer registry has " + QString::number( QgsProject::instance()->count() ) + "layers" );

  //metadata();

  // Use the affine transform to get geo coordinates for
  // the corners of the raster
  double myXMax = mGeoTransform[0] +
                  GDALGetRasterXSize( mGdalDataset ) * mGeoTransform[1] +
                  GDALGetRasterYSize( mGdalDataset ) * mGeoTransform[2];
  double myYMin = mGeoTransform[3] +
                  GDALGetRasterXSize( mGdalDataset ) * mGeoTransform[4] +
                  GDALGetRasterYSize( mGdalDataset ) * mGeoTransform[5];

  mExtent.setXMaximum( myXMax );
  // The affine transform reduces to these values at the
  // top-left corner of the raster
  mExtent.setXMinimum( mGeoTransform[0] );
  mExtent.setYMaximum( mGeoTransform[3] );
  mExtent.setYMinimum( myYMin );

  //
  // Set up the x and y dimensions of this raster layer
  //
  mWidth = GDALGetRasterXSize( mGdalDataset );
  mHeight = GDALGetRasterYSize( mGdalDataset );

  // Check if the dataset has a mask band, that applies to the whole dataset
  // If so then expose it as an alpha band.
  int nMaskFlags = GDALGetMaskFlags( myGDALBand );
  const int bandCount = GDALGetRasterCount( mGdalDataset );
  if ( ( nMaskFlags == 0 && bandCount == 1 ) || nMaskFlags == GMF_PER_DATASET )
  {
    mMaskBandExposedAsAlpha = true;
  }

  mBandCount = bandCount + ( mMaskBandExposedAsAlpha ? 1 : 0 );

  GDALGetBlockSize( GDALGetRasterBand( mGdalDataset, 1 ), &mXBlockSize, &mYBlockSize );
  //
  // Determine the nodata value and data type
  //
  //mValidNoDataValue = true;
  for ( int i = 1; i <= bandCount; i++ )
  {
    GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, i );
    GDALDataType myGdalDataType = GDALGetRasterDataType( myGdalBand );

    int isValid = false;
    double myNoDataValue = GDALGetRasterNoDataValue( myGdalBand, &isValid );
    // We check that the double value we just got is representable in the
    // data type. In normal situations this should not be needed, but it happens
    // to have 8bit TIFFs with nan as the nodata value. If not checking against
    // the min/max bounds, it would be cast to 0 by representableValue().
    if ( isValid && !QgsRaster::isRepresentableValue( myNoDataValue, dataTypeFromGdal( myGdalDataType ) ) )
    {
      QgsDebugMsg( QStringLiteral( "GDALGetRasterNoDataValue = %1 is not representable in data type, so ignoring it" ).arg( myNoDataValue ) );
      isValid = false;
    }
    if ( isValid )
    {
      QgsDebugMsg( QStringLiteral( "GDALGetRasterNoDataValue = %1" ).arg( myNoDataValue ) );
      // The no data value double may be non representable by data type, it can result
      // in problems if that value is used to represent additional user defined no data
      // see #3840
      myNoDataValue = QgsRaster::representableValue( myNoDataValue, dataTypeFromGdal( myGdalDataType ) );
      mSrcNoDataValue.append( myNoDataValue );
      mSrcHasNoDataValue.append( true );
      mUseSrcNoDataValue.append( true );
    }
    else
    {
      mSrcNoDataValue.append( std::numeric_limits<double>::quiet_NaN() );
      mSrcHasNoDataValue.append( false );
      mUseSrcNoDataValue.append( false );
    }
    // It may happen that nodata value given by GDAL is wrong and it has to be
    // disabled by user, in that case we need another value to be used for nodata
    // (for reprojection for example) -> always internaly represent as wider type
    // with mInternalNoDataValue in reserve.
    // Not used
#if 0
    int myInternalGdalDataType = myGdalDataType;
    double myInternalNoDataValue = 123;
    switch ( srcDataType( i ) )
    {
      case Qgis::Byte:
        myInternalNoDataValue = -32768.0;
        myInternalGdalDataType = GDT_Int16;
        break;
      case Qgis::Int16:
        myInternalNoDataValue = -2147483648.0;
        myInternalGdalDataType = GDT_Int32;
        break;
      case Qgis::UInt16:
        myInternalNoDataValue = -2147483648.0;
        myInternalGdalDataType = GDT_Int32;
        break;
      case Qgis::Int32:
        // We believe that such values is no used in real data
        myInternalNoDataValue = -2147483648.0;
        break;
      case Qgis::UInt32:
        // We believe that such values is no used in real data
        myInternalNoDataValue = 4294967295.0;
        break;
      default: // Float32, Float64
        //myNoDataValue = std::numeric_limits<int>::max();
        // NaN should work well
        myInternalNoDataValue = std::numeric_limits<double>::quiet_NaN();
    }
#endif
    //mGdalDataType.append( myInternalGdalDataType );

    // define if the band has scale and offset to apply
    double myScale = bandScale( i );
    double myOffset = bandOffset( i );
    if ( !qgsDoubleNear( myScale, 1.0 ) || !qgsDoubleNear( myOffset, 0.0 ) )
    {
      // if the band has scale or offset to apply change dataType
      switch ( myGdalDataType )
      {
        case GDT_Unknown:
        case GDT_TypeCount:
          break;
        case GDT_Byte:
        case GDT_UInt16:
        case GDT_Int16:
        case GDT_UInt32:
        case GDT_Int32:
        case GDT_Float32:
        case GDT_CInt16:
          myGdalDataType = GDT_Float32;
          break;
        case GDT_Float64:
        case GDT_CInt32:
        case GDT_CFloat32:
          myGdalDataType = GDT_Float64;
          break;
        case GDT_CFloat64:
          break;
      }
    }

    mGdalDataType.append( myGdalDataType );
    //mInternalNoDataValue.append( myInternalNoDataValue );
    //QgsDebugMsg( QStringLiteral( "mInternalNoDataValue[%1] = %2" ).arg( i - 1 ).arg( mInternalNoDataValue[i-1] ) );
  }

  if ( mMaskBandExposedAsAlpha )
  {
    mSrcNoDataValue.append( std::numeric_limits<double>::quiet_NaN() );
    mSrcHasNoDataValue.append( false );
    mUseSrcNoDataValue.append( false );
    mGdalDataType.append( GDT_Byte );
  }
}

char **papszFromStringList( const QStringList &list )
{
  char **papszRetList = nullptr;
  Q_FOREACH ( const QString &elem, list )
  {
    papszRetList = CSLAddString( papszRetList, elem.toLocal8Bit().constData() );
  }
  return papszRetList;
}

#if 0
bool QgsGdalProvider::create( const QString &format, int nBands,
                              Qgis::DataType type,
                              int width, int height, double *geoTransform,
                              const QgsCoordinateReferenceSystem &crs,
                              QStringList createOptions )
#endif
QGISEXTERN QgsGdalProvider *create(
  const QString &uri,
  const QString &format, int nBands,
  Qgis::DataType type,
  int width, int height, double *geoTransform,
  const QgsCoordinateReferenceSystem &crs,
  QStringList createOptions )
{
  //get driver
  GDALDriverH driver = GDALGetDriverByName( format.toLocal8Bit().data() );
  if ( !driver )
  {
    QgsError error( "Cannot load GDAL driver " + format, QStringLiteral( "GDAL provider" ) );
    return new QgsGdalProvider( uri, error );
  }

  QgsDebugMsg( "create options: " + createOptions.join( " " ) );

  //create dataset
  CPLErrorReset();
  char **papszOptions = papszFromStringList( createOptions );
  gdal::dataset_unique_ptr dataset( GDALCreate( driver, uri.toUtf8().constData(), width, height, nBands, ( GDALDataType )type, papszOptions ) );
  CSLDestroy( papszOptions );
  if ( !dataset )
  {
    QgsError error( QStringLiteral( "Cannot create new dataset  %1:\n%2" ).arg( uri, QString::fromUtf8( CPLGetLastErrorMsg() ) ), QStringLiteral( "GDAL provider" ) );
    QgsDebugMsg( error.summary() );
    return new QgsGdalProvider( uri, error );
  }

  GDALSetGeoTransform( dataset.get(), geoTransform );
  GDALSetProjection( dataset.get(), crs.toWkt().toLocal8Bit().data() );

  QgsDataProvider::ProviderOptions providerOptions;
  return new QgsGdalProvider( uri, providerOptions, true, dataset.release() );
}

bool QgsGdalProvider::write( void *data, int band, int width, int height, int xOffset, int yOffset )
{
  QMutexLocker locker( mpMutex );
  if ( !initIfNeeded() )
    return false;

  if ( !mGdalDataset )
  {
    return false;
  }
  GDALRasterBandH rasterBand = getBand( band );
  if ( !rasterBand )
  {
    return false;
  }
  return gdalRasterIO( rasterBand, GF_Write, xOffset, yOffset, width, height, data, width, height, GDALGetRasterDataType( rasterBand ), 0, 0 ) == CE_None;
}

bool QgsGdalProvider::setNoDataValue( int bandNo, double noDataValue )
{
  QMutexLocker locker( mpMutex );
  if ( !initIfNeeded() )
    return false;

  if ( !mGdalDataset )
  {
    return false;
  }

  GDALRasterBandH rasterBand = getBand( bandNo );
  CPLErrorReset();
  CPLErr err = GDALSetRasterNoDataValue( rasterBand, noDataValue );
  if ( err != CPLE_None )
  {
    QgsDebugMsg( QStringLiteral( "Cannot set no data value" ) );
    return false;
  }
  mSrcNoDataValue[bandNo - 1] = noDataValue;
  mSrcHasNoDataValue[bandNo - 1] = true;
  mUseSrcNoDataValue[bandNo - 1] = true;
  return true;
}

bool QgsGdalProvider::remove()
{
  QMutexLocker locker( mpMutex );
  if ( !initIfNeeded() )
    return false;

  while ( *mpRefCounter != 1 )
  {
    QgsDebugMsg( QStringLiteral( "Waiting for ref counter for %1 to drop to 1" ).arg( dataSourceUri() ) );
    QThread::msleep( 100 );
  }

  if ( mGdalDataset )
  {
    GDALDriverH driver = GDALGetDatasetDriver( mGdalDataset );
    closeDataset();

    CPLErrorReset();
    CPLErr err = GDALDeleteDataset( driver, dataSourceUri( true ).toUtf8().constData() );
    if ( err != CPLE_None )
    {
      QgsLogger::warning( "RasterIO error: " + QString::fromUtf8( CPLGetLastErrorMsg() ) );
      QgsDebugMsg( "RasterIO error: " + QString::fromUtf8( CPLGetLastErrorMsg() ) );
      return false;
    }
    QgsDebugMsg( QStringLiteral( "Raster dataset dataSourceUri() successfully deleted" ) );
    return true;
  }
  return false;
}

/**
  Builds the list of file filter strings to later be used by
  QgisApp::addRasterLayer()

  We query GDAL for a list of supported raster formats; we then build
  a list of file filter strings from that list.  We return a string
  that contains this list that is suitable for use in a
  QFileDialog::getOpenFileNames() call.

*/
QGISEXTERN void buildSupportedRasterFileFilter( QString &fileFiltersString )
{
  QStringList exts;
  QStringList wildcards;
  buildSupportedRasterFileFilterAndExtensions( fileFiltersString, exts, wildcards );
}

/**
  Gets creation options metadata for a given format
*/
QGISEXTERN QString helpCreationOptionsFormat( QString format )
{
  QString message;
  GDALDriverH myGdalDriver = GDALGetDriverByName( format.toLocal8Bit().constData() );
  if ( myGdalDriver )
  {
    // first report details and help page
    char **GDALmetadata = GDALGetMetadata( myGdalDriver, nullptr );
    message += QLatin1String( "Format Details:\n" );
    message += QStringLiteral( "  Extension: %1\n" ).arg( CSLFetchNameValue( GDALmetadata, GDAL_DMD_EXTENSION ) );
    message += QStringLiteral( "  Short Name: %1" ).arg( GDALGetDriverShortName( myGdalDriver ) );
    message += QStringLiteral( "  /  Long Name: %1\n" ).arg( GDALGetDriverLongName( myGdalDriver ) );
    message += QStringLiteral( "  Help page:  http://www.gdal.org/%1\n\n" ).arg( CSLFetchNameValue( GDALmetadata, GDAL_DMD_HELPTOPIC ) );

    // next get creation options
    // need to serialize xml to get newlines, should we make the basic xml prettier?
    CPLXMLNode *psCOL = CPLParseXMLString( GDALGetMetadataItem( myGdalDriver,
                                           GDAL_DMD_CREATIONOPTIONLIST, "" ) );
    char *pszFormattedXML = CPLSerializeXMLTree( psCOL );
    if ( pszFormattedXML )
      message += QString( pszFormattedXML );
    if ( psCOL )
      CPLDestroyXMLNode( psCOL );
    if ( pszFormattedXML )
      CPLFree( pszFormattedXML );
  }
  return message;
}

/**
  Validates creation options for a given format, regardless of layer.
*/
QGISEXTERN QString validateCreationOptionsFormat( const QStringList &createOptions, QString format )
{
  GDALDriverH myGdalDriver = GDALGetDriverByName( format.toLocal8Bit().constData() );
  if ( ! myGdalDriver )
    return QStringLiteral( "invalid GDAL driver" );

  char **papszOptions = papszFromStringList( createOptions );
  // get error string?
  int ok = GDALValidateCreationOptions( myGdalDriver, papszOptions );
  CSLDestroy( papszOptions );

  if ( !ok )
    return QStringLiteral( "Failed GDALValidateCreationOptions() test" );
  return QString();
}

QString QgsGdalProvider::validateCreationOptions( const QStringList &createOptions, const QString &format )
{
  QString message;

  // first validate basic syntax with GDALValidateCreationOptions
  message = validateCreationOptionsFormat( createOptions, format );
  if ( !message.isNull() )
    return message;

  // next do specific validations, depending on format and dataset
  // only check certain destination formats
  QStringList formatsCheck;
  formatsCheck << QStringLiteral( "gtiff" );
  if ( ! formatsCheck.contains( format.toLower() ) )
    return QString();

  // prepare a map for easier lookup
  QMap< QString, QString > optionsMap;
  Q_FOREACH ( const QString &option, createOptions )
  {
    QStringList opt = option.split( '=' );
    optionsMap[ opt[0].toUpper()] = opt[1];
    QgsDebugMsg( "option: " + option );
  }

  // gtiff files - validate PREDICTOR option
  // see gdal: frmts/gtiff/geotiff.cpp and libtiff: tif_predict.c)
  if ( format.compare( QLatin1String( "gtiff" ), Qt::CaseInsensitive ) == 0 && optionsMap.contains( QStringLiteral( "PREDICTOR" ) ) )
  {
    QString value = optionsMap.value( QStringLiteral( "PREDICTOR" ) );
    GDALDataType nDataType = ( !mGdalDataType.isEmpty() ) ? ( GDALDataType ) mGdalDataType.at( 0 ) : GDT_Unknown;
    int nBitsPerSample = nDataType != GDT_Unknown ? GDALGetDataTypeSize( nDataType ) : 0;
    QgsDebugMsg( QStringLiteral( "PREDICTOR: %1 nbits: %2 type: %3" ).arg( value ).arg( nBitsPerSample ).arg( ( GDALDataType ) mGdalDataType.at( 0 ) ) );
    // PREDICTOR=2 only valid for 8/16/32 bits per sample
    // TODO check for NBITS option (see geotiff.cpp)
    if ( value == QLatin1String( "2" ) )
    {
      if ( nBitsPerSample != 8 && nBitsPerSample != 16 &&
           nBitsPerSample != 32 )
      {
        message = QStringLiteral( "PREDICTOR=%1 only valid for 8/16/32 bits per sample (using %2)" ).arg( value ).arg( nBitsPerSample );
      }
    }
    // PREDICTOR=3 only valid for float/double precision
    else if ( value == QLatin1String( "3" ) )
    {
      if ( nDataType != GDT_Float32 && nDataType != GDT_Float64 )
        message = QStringLiteral( "PREDICTOR=3 only valid for float/double precision" );
    }
  }

  return message;
}

QString QgsGdalProvider::validatePyramidsConfigOptions( QgsRaster::RasterPyramidsFormat pyramidsFormat,
    const QStringList &configOptions, const QString &fileFormat )
{
  // Erdas Imagine format does not support config options
  if ( pyramidsFormat == QgsRaster::PyramidsErdas )
  {
    if ( ! configOptions.isEmpty() )
      return QStringLiteral( "Erdas Imagine format does not support config options" );
    else
      return QString();
  }
  // Internal pyramids format only supported for gtiff/georaster/hfa/jp2kak/mrsid/nitf files
  else if ( pyramidsFormat == QgsRaster::PyramidsInternal )
  {
    QStringList supportedFormats;
    supportedFormats << QStringLiteral( "gtiff" ) << QStringLiteral( "georaster" ) << QStringLiteral( "hfa" ) << QStringLiteral( "gpkg" ) << QStringLiteral( "rasterlite" ) << QStringLiteral( "nitf" );
    if ( ! supportedFormats.contains( fileFormat.toLower() ) )
      return QStringLiteral( "Internal pyramids format only supported for gtiff/georaster/gpkg/rasterlite/nitf files (using %1)" ).arg( fileFormat );
  }
  else
  {
    // for gtiff external pyramids, validate gtiff-specific values
    // PHOTOMETRIC_OVERVIEW=YCBCR requires a source raster with only 3 bands (RGB)
    if ( configOptions.contains( QStringLiteral( "PHOTOMETRIC_OVERVIEW=YCBCR" ) ) )
    {
      if ( GDALGetRasterCount( mGdalDataset ) != 3 )
        return QStringLiteral( "PHOTOMETRIC_OVERVIEW=YCBCR requires a source raster with only 3 bands (RGB)" );
    }
  }

  return QString();
}

bool QgsGdalProvider::isEditable() const
{
  return mUpdate;
}

bool QgsGdalProvider::setEditable( bool enabled )
{
  QMutexLocker locker( mpMutex );
  if ( !initIfNeeded() )
    return false;

  if ( enabled == mUpdate )
    return false;

  if ( !mValid )
    return false;

  if ( mGdalDataset != mGdalBaseDataset )
    return false;  // ignore the case of warped VRT for now (more complicated setup)

  while ( *mpRefCounter != 1 )
  {
    QgsDebugMsg( QStringLiteral( "Waiting for ref counter for %1 to drop to 1" ).arg( dataSourceUri() ) );
    QThread::msleep( 100 );
  }

  closeDataset();

  mUpdate = enabled;

  // reopen the dataset
  mGdalBaseDataset = gdalOpen( dataSourceUri( true ).toUtf8().constData(), mUpdate ? GA_Update : GA_ReadOnly );
  if ( !mGdalBaseDataset )
  {
    QString msg = QStringLiteral( "Cannot reopen GDAL dataset %1:\n%2" ).arg( dataSourceUri(), QString::fromUtf8( CPLGetLastErrorMsg() ) );
    appendError( ERRMSG( msg ) );
    return false;
  }

  //Since we are not a virtual warped dataset, mGdalDataSet and mGdalBaseDataset are supposed to be the same
  mGdalDataset = mGdalBaseDataset;
  mValid = true;
  return true;
}

GDALRasterBandH QgsGdalProvider::getBand( int bandNo ) const
{
  QMutexLocker locker( mpMutex );
  if ( !const_cast<QgsGdalProvider *>( this )->initIfNeeded() )
    return nullptr;

  if ( mMaskBandExposedAsAlpha && bandNo == GDALGetRasterCount( mGdalDataset ) + 1 )
    return GDALGetMaskBand( GDALGetRasterBand( mGdalDataset, 1 ) );
  else
    return GDALGetRasterBand( mGdalDataset, bandNo );
}

// pyramids resampling

// see http://www.gdal.org/gdaladdo.html
//     http://www.gdal.org/classGDALDataset.html#a2aa6f88b3bbc840a5696236af11dde15
//     http://www.gdal.org/classGDALRasterBand.html#afaea945b13ec9c86c2d783b883c68432

// from http://www.gdal.org/gdaladdo.html
//   average_mp is unsuitable for use thus not included

// from qgsgdalprovider.cpp (removed)
//   NOTE magphase is disabled in the gui since it tends
//   to create corrupted images. The images can be repaired
//   by running one of the other resampling strategies below.
//   see ticket #284

QGISEXTERN QList<QPair<QString, QString> > *pyramidResamplingMethods()
{
  static QList<QPair<QString, QString> > methods;

  if ( methods.isEmpty() )
  {
    methods.append( QPair<QString, QString>( QStringLiteral( "NEAREST" ), QObject::tr( "Nearest Neighbour" ) ) );
    methods.append( QPair<QString, QString>( QStringLiteral( "AVERAGE" ), QObject::tr( "Average" ) ) );
    methods.append( QPair<QString, QString>( QStringLiteral( "GAUSS" ), QObject::tr( "Gauss" ) ) );
    methods.append( QPair<QString, QString>( QStringLiteral( "CUBIC" ), QObject::tr( "Cubic" ) ) );
    methods.append( QPair<QString, QString>( QStringLiteral( "CUBICSPLINE" ), QObject::tr( "Cubic Spline" ) ) );
    methods.append( QPair<QString, QString>( QStringLiteral( "LANCZOS" ), QObject::tr( "Lanczos" ) ) );
    methods.append( QPair<QString, QString>( QStringLiteral( "MODE" ), QObject::tr( "Mode" ) ) );
    methods.append( QPair<QString, QString>( QStringLiteral( "NONE" ), QObject::tr( "None" ) ) );
  }

  return &methods;
}

QGISEXTERN void cleanupProvider()
{
  // nothing to do here, QgsApplication takes care of
  // calling GDALDestroyDriverManager()
}

QGISEXTERN QList< QgsDataItemProvider * > *dataItemProviders()
{
  QList< QgsDataItemProvider * > *providers = new QList< QgsDataItemProvider * >();
  *providers << new QgsGdalDataItemProvider;
  return providers;
}

#ifdef HAVE_GUI

//! Provider for gdal raster source select
class QgsGdalRasterSourceSelectProvider : public QgsSourceSelectProvider
{
  public:

    QString providerKey() const override { return QStringLiteral( "gdal" ); }
    QString text() const override { return QObject::tr( "Raster" ); }
    int ordering() const override { return QgsSourceSelectProvider::OrderLocalProvider + 20; }
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddRasterLayer.svg" ) ); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsGdalSourceSelect( parent, fl, widgetMode );
    }
};


QGISEXTERN QList<QgsSourceSelectProvider *> *sourceSelectProviders()
{
  QList<QgsSourceSelectProvider *> *providers = new QList<QgsSourceSelectProvider *>();

  *providers
      << new QgsGdalRasterSourceSelectProvider;

  return providers;
}

#endif
