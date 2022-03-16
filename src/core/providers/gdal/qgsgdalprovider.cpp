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

#include "qgsgdalprovider.h"
///@cond PRIVATE

#include "qgslogger.h"
#include "qgsgdalproviderbase.h"
#include "qgsgdalprovider.h"
#include "qgsconfig.h"

#include "qgsgdalutils.h"
#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgscoordinatetransform.h"
#include "qgsdataitemprovider.h"
#include "qgsdatasourceuri.h"
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
#include "qgsruntimeprofiler.h"
#include "qgszipitem.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsproviderutils.h"

#include <QImage>
#include <QColor>
#include <QProcess>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QHash>
#include <QTime>
#include <QTextDocument>
#include <QDebug>
#include <QRegularExpression>

#include <gdalwarper.h>
#include <gdal.h>
#include <ogr_srs_api.h>
#include <cpl_conv.h>
#include <cpl_string.h>

#define ERRMSG(message) QGS_ERROR_MESSAGE(message,"GDAL provider")
#define ERR(message) QgsError(message,"GDAL provider")

#define PROVIDER_KEY QStringLiteral( "gdal" )
#define PROVIDER_DESCRIPTION QStringLiteral( "GDAL data provider" )

// To avoid potential races when destroying related instances ("main" and clones)
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
Q_GLOBAL_STATIC_WITH_ARGS( QMutex, sGdalProviderMutex, ( QMutex::Recursive ) )
#else
Q_GLOBAL_STATIC( QRecursiveMutex, sGdalProviderMutex )
#endif

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
  Q_UNUSED( pszMessage )

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
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  , mpMutex( new QMutex( QMutex::Recursive ) )
#else
  , mpMutex( new QRecursiveMutex() )
#endif
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

  QgsDebugMsgLevel( "constructing with uri '" + uri + "'.", 2 );

  QgsGdalProviderBase::registerGdalDrivers();

#ifndef QT_NO_NETWORKPROXY
  QgsGdalUtils::setupProxy();
#endif

  std::unique_ptr< QgsScopedRuntimeProfile > profile;
  if ( QgsApplication::profiler()->groupIsActive( QStringLiteral( "projectload" ) ) )
    profile = std::make_unique< QgsScopedRuntimeProfile >( tr( "Open data source" ), QStringLiteral( "projectload" ) );

  if ( !CPLGetConfigOption( "AAIGRID_DATATYPE", nullptr ) )
  {
    // GDAL tends to open AAIGrid as Float32 which results in lost precision
    // and confusing values shown to users, force Float64
    CPLSetConfigOption( "AAIGRID_DATATYPE", "Float64" );
  }

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
  mDriverName = other.mDriverName;

  // The JP2OPENJPEG driver might consume too much memory on large datasets
  // so make sure to really use a single one.
  // The PostGISRaster driver internally uses a per-thread connection cache.
  // This can lead to crashes if two datasets created by the same thread are used at the same time.
  bool forceUseSameDataset = ( mDriverName.toUpper() == QLatin1String( "JP2OPENJPEG" ) ||
                               mDriverName == QLatin1String( "PostGISRaster" ) ||
                               CSLTestBoolean( CPLGetConfigOption( "QGIS_GDAL_FORCE_USE_SAME_DATASET", "FALSE" ) ) );

  if ( forceUseSameDataset )
  {
    ++ ( *other.mpRefCounter );
    // cppcheck-suppress copyCtorPointerCopying
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
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    mpMutex = new QMutex( QMutex::Recursive );
#else
    mpMutex = new QRecursiveMutex();
#endif

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
    return QgsGdalProvider::expandAuthConfig( QgsDataProvider::dataSourceUri() );
  }
  else
  {
    return QgsDataProvider::dataSourceUri();
  }
}

QString QgsGdalProvider::expandAuthConfig( const QString &dsName )
{
  QString uri( dsName );
  // Check for authcfg
  QRegularExpression authcfgRe( " authcfg='([^']+)'" );
  QRegularExpressionMatch match;
  if ( uri.contains( authcfgRe, &match ) )
  {
    uri = uri.remove( match.captured( 0 ) );
    QString configId( match.captured( 1 ) );
    QStringList connectionItems;
    connectionItems << uri;
    if ( QgsApplication::authManager()->updateDataSourceUriItems( connectionItems, configId, QStringLiteral( "ogr" ) ) )
    {
      uri = connectionItems.first( );
    }
  }
  return uri;
}

QgsGdalProvider *QgsGdalProvider::clone() const
{
  return new QgsGdalProvider( *this );
}

bool QgsGdalProvider::getCachedGdalHandles( QgsGdalProvider *provider,
    GDALDatasetH &gdalBaseDataset,
    GDALDatasetH &gdalDataset )
{
  QMutexLocker locker( sGdalProviderMutex() );

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
  QMutexLocker locker( sGdalProviderMutex() );

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
  QMutexLocker locker( sGdalProviderMutex() );
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
  QMutexLocker locker( sGdalProviderMutex() );

  if ( mGdalTransformerArg )
    GDALDestroyTransformer( mGdalTransformerArg );

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

void QgsGdalProvider::reloadProviderData()
{
  QMutexLocker locker( mpMutex );
  closeDataset();

  mHasInit = false;
  ( void )initIfNeeded();
}

QString QgsGdalProvider::htmlMetadata()
{
  QMutexLocker locker( mpMutex );
  if ( !initIfNeeded() )
    return QString();

  GDALDriverH hDriver = GDALGetDriverByName( mDriverName.toLocal8Bit().constData() );
  if ( !hDriver )
    return QString();

  QString myMetadata;

  // GDAL Driver description
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "GDAL Driver Description" ) + QStringLiteral( "</td><td>" ) + mDriverName + QStringLiteral( "</td></tr>\n" );

  // GDAL Driver Metadata
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "GDAL Driver Metadata" ) + QStringLiteral( "</td><td>" ) + QString( GDALGetMetadataItem( hDriver, GDAL_DMD_LONGNAME, nullptr ) ) + QStringLiteral( "</td></tr>\n" );

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
    myMetadata += QgsHtmlUtils::buildBulletList( QStringList(
    {
      QObject::tr( "Scale: %1" ).arg( bandScale( i ) ),
      QObject::tr( "Offset: %1" ).arg( bandOffset( i ) ),
    } ) );
    myMetadata += QLatin1String( "</td></tr>" );
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
  myMetadata += QLatin1String( "</td></tr>\n" );

  // Dimensions
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Dimensions" ) + QStringLiteral( "</td><td>" );
  myMetadata += tr( "X: %1 Y: %2 Bands: %3" )
                .arg( GDALGetRasterXSize( mGdalDataset ) )
                .arg( GDALGetRasterYSize( mGdalDataset ) )
                .arg( GDALGetRasterCount( mGdalDataset ) );
  myMetadata += QLatin1String( "</td></tr>\n" );

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
  std::unique_ptr< QgsRasterBlock > block = std::make_unique< QgsRasterBlock >( dataType( bandNo ), width, height );
  if ( !initIfNeeded() )
    return block.release();
  if ( sourceHasNoDataValue( bandNo ) && useSourceNoDataValue( bandNo ) )
  {
    block->setNoDataValue( sourceNoDataValue( bandNo ) );
  }

  if ( block->isEmpty() )
  {
    return block.release();
  }

  if ( !mExtent.intersects( extent ) )
  {
    // the requested extent is completely outside of the raster's extent - nothing to do
    block->setIsNoData();
    return block.release();
  }

  if ( !mExtent.contains( extent ) )
  {
    QRect subRect = QgsRasterBlock::subRect( extent, width, height, mExtent );
    block->setIsNoDataExcept( subRect );
  }
  if ( !readBlock( bandNo, extent, width, height, block->bits(), feedback ) )
  {
    block->setError( { tr( "Error occurred while reading block." ), QStringLiteral( "GDAL" ) } );
    block->setIsNoData();
    block->setValid( false );
    return block.release();
  }
  // apply scale and offset
  Q_ASSERT( block ); // to make cppcheck happy
  block->applyScaleOffset( bandScale( bandNo ), bandOffset( bandNo ) );
  block->applyNoDataValues( userNoDataValues( bandNo ) );
  return block.release();
}

bool QgsGdalProvider::readBlock( int bandNo, int xBlock, int yBlock, void *data )
{
  QMutexLocker locker( mpMutex );
  if ( !initIfNeeded() )
    return false;

  // TODO!!!: Check data alignment!!! May it happen that nearest value which
  // is not nearest is assigned to an output cell???

  GDALRasterBandH myGdalBand = getBand( bandNo );
  //GDALReadBlock( myGdalBand, xBlock, yBlock, block );

  // We have to read with correct data type consistent with other readBlock functions
  int xOff = xBlock * mXBlockSize;
  int yOff = yBlock * mYBlockSize;
  CPLErr err = gdalRasterIO( myGdalBand, GF_Read, xOff, yOff, mXBlockSize, mYBlockSize, data, mXBlockSize, mYBlockSize, ( GDALDataType ) mGdalDataType.at( bandNo - 1 ), 0, 0 );
  if ( err != CPLE_None )
  {
    QgsLogger::warning( "RasterIO error: " + QString::fromUtf8( CPLGetLastErrorMsg() ) );
    return false;
  }

  return true;
}

bool QgsGdalProvider::canDoResampling(
  int bandNo,
  const QgsRectangle &reqExtent,
  int bufferWidthPix,
  int bufferHeightPix )
{
  QMutexLocker locker( mpMutex );
  if ( !initIfNeeded() )
    return false;

  GDALRasterBandH gdalBand = getBand( bandNo );
  if ( GDALGetRasterColorTable( gdalBand ) )
    return false;

  const double reqXRes = reqExtent.width() / bufferWidthPix;
  const double reqYRes = reqExtent.height() / bufferHeightPix;
  const double srcXRes = mGeoTransform[1];
  const double srcYRes = fabs( mGeoTransform[5] );
  // Compute the resampling factor :
  // < 1 means upsampling / zoom-in
  // > 1 means downsampling / zoom-out
  const double resamplingFactor = std::max( reqXRes / srcXRes, reqYRes / srcYRes );

  if ( resamplingFactor < 1 )
  {
    // upsampling
    return mZoomedInResamplingMethod != QgsRasterDataProvider::ResamplingMethod::Nearest;
  }

  if ( resamplingFactor < 1.1 )
  {
    // very close to nominal resolution ==> check compatibility of zoom-in or zoom-out resampler with what GDAL can do
    return mZoomedInResamplingMethod != QgsRasterDataProvider::ResamplingMethod::Nearest ||
           mZoomedOutResamplingMethod != QgsRasterDataProvider::ResamplingMethod::Nearest;
  }

  // if no zoom out resampling, exit now
  if ( mZoomedOutResamplingMethod == QgsRasterDataProvider::ResamplingMethod::Nearest )
  {
    return false;
  }

  // if the resampling factor is not too large, we can do the downsampling
  // from the full resolution with acceptable performance
  if ( resamplingFactor <= ( mMaxOversampling + .1 ) )
  {
    return true;
  }

  // otherwise, we have to check that we have an overview band that satisfies
  // that criterion
  const int nOvrCount = GDALGetOverviewCount( gdalBand );
  for ( int i = 0; i < nOvrCount; i++ )
  {
    GDALRasterBandH hOvrBand = GDALGetOverview( gdalBand, i );
    const double ovrResamplingFactor = xSize() / static_cast<double>( GDALGetRasterBandXSize( hOvrBand ) );
    if ( resamplingFactor <= ( mMaxOversampling + .1 ) * ovrResamplingFactor )
    {
      return true;
    }
  }

  // too much zoomed out
  return false;
}

static GDALRIOResampleAlg getGDALResamplingAlg( QgsGdalProvider::ResamplingMethod method )
{
  GDALRIOResampleAlg eResampleAlg = GRIORA_NearestNeighbour;
  switch ( method )
  {
    case QgsGdalProvider::ResamplingMethod::Nearest:
      eResampleAlg = GRIORA_NearestNeighbour;
      break;

    case QgsGdalProvider::ResamplingMethod::Bilinear:
      eResampleAlg = GRIORA_Bilinear;
      break;

    case QgsGdalProvider::ResamplingMethod::Cubic:
      eResampleAlg = GRIORA_Cubic;
      break;

    case QgsRasterDataProvider::ResamplingMethod::CubicSpline:
      eResampleAlg = GRIORA_CubicSpline;
      break;

    case QgsRasterDataProvider::ResamplingMethod::Lanczos:
      eResampleAlg = GRIORA_Lanczos;
      break;

    case QgsRasterDataProvider::ResamplingMethod::Average:
      eResampleAlg = GRIORA_Average;
      break;

    case QgsRasterDataProvider::ResamplingMethod::Mode:
      eResampleAlg = GRIORA_Mode;
      break;

    case QgsRasterDataProvider::ResamplingMethod::Gauss:
      eResampleAlg = GRIORA_Gauss;
      break;
  }

  return eResampleAlg;
}

bool QgsGdalProvider::readBlock( int bandNo, QgsRectangle  const &reqExtent, int bufferWidthPix, int bufferHeightPix, void *data, QgsRasterBlockFeedback *feedback )
{
  QMutexLocker locker( mpMutex );
  if ( !initIfNeeded() )
    return false;

  QgsDebugMsgLevel( "bufferWidthPix = "  + QString::number( bufferWidthPix ), 5 );
  QgsDebugMsgLevel( "bufferHeightPix = "  + QString::number( bufferHeightPix ), 5 );
  QgsDebugMsgLevel( "reqExtent: " + reqExtent.toString(), 5 );

  for ( int i = 0; i < 6; i++ )
  {
    QgsDebugMsgLevel( QStringLiteral( "transform : %1" ).arg( mGeoTransform[i] ), 5 );
  }

  const size_t dataSize = static_cast<size_t>( dataTypeSize( bandNo ) );

  QgsRectangle intersectExtent = reqExtent.intersect( mExtent );
  if ( intersectExtent.isEmpty() )
  {
    QgsDebugMsgLevel( QStringLiteral( "draw request outside view extent." ), 2 );
    return false;
  }
  QgsDebugMsgLevel( "extent: " + mExtent.toString(), 5 );
  QgsDebugMsgLevel( "intersectExtent: " + intersectExtent.toString(), 5 );

  const double reqXRes = reqExtent.width() / bufferWidthPix;
  const double reqYRes = reqExtent.height() / bufferHeightPix;

  const double srcXRes = mGeoTransform[1];
  const double srcYRes = mGeoTransform[5]; // may be negative?
  QgsDebugMsgLevel( QStringLiteral( "reqXRes = %1 reqYRes = %2 srcXRes = %3 srcYRes = %4" ).arg( reqXRes ).arg( reqYRes ).arg( srcXRes ).arg( srcYRes ), 5 );
  const double resamplingFactor = std::max( reqXRes / srcXRes, reqYRes / srcYRes );

  GDALRasterBandH gdalBand = getBand( bandNo );
  const GDALDataType type = static_cast<GDALDataType>( mGdalDataType.at( bandNo - 1 ) );

  // Find top, bottom rows and left, right column the raster extent covers
  // These are limits in target grid space
  QRect subRect = QgsRasterBlock::subRect( reqExtent, bufferWidthPix, bufferHeightPix, intersectExtent );
  const int tgtTopOri = subRect.top();
  const int tgtBottomOri = subRect.bottom();
  const int tgtLeftOri = subRect.left();
  const int tgtRightOri = subRect.right();

  // Calculate rows/cols limits in source raster grid space
  int srcLeft = 0;
  int srcTop = 0;
  int srcBottom = ySize() - 1;
  int srcRight = xSize() - 1;

  // Get necessary src extent aligned to src resolution
  if ( mExtent.xMinimum() < intersectExtent.xMinimum() )
  {
    srcLeft = static_cast<int>( std::floor( ( intersectExtent.xMinimum() - mExtent.xMinimum() ) / srcXRes ) );
  }
  if ( mExtent.xMaximum() > intersectExtent.xMaximum() )
  {
    // Clamp to raster width to avoid potential rounding errors (see GH #34435)
    srcRight = std::min( mWidth - 1, static_cast<int>( std::floor( ( intersectExtent.xMaximum() - mExtent.xMinimum() ) / srcXRes ) ) );
  }

  // GDAL states that mGeoTransform[3] is top, may it also be bottom and mGeoTransform[5] positive?
  if ( mExtent.yMaximum() > intersectExtent.yMaximum() )
  {
    srcTop = static_cast<int>( std::floor( -1. * ( mExtent.yMaximum() - intersectExtent.yMaximum() ) / srcYRes ) );
  }
  if ( mExtent.yMinimum() < intersectExtent.yMinimum() )
  {
    // Clamp to raster height to avoid potential rounding errors (see GH #34435)
    srcBottom = std::min( mHeight - 1, static_cast<int>( std::floor( -1. * ( mExtent.yMaximum() - intersectExtent.yMinimum() ) / srcYRes ) ) );
  }

  const int srcWidth = srcRight - srcLeft + 1;
  const int srcHeight = srcBottom - srcTop + 1;

  // Use GDAL resampling if asked and possible
  if ( mProviderResamplingEnabled &&
       canDoResampling( bandNo, reqExtent, bufferWidthPix, bufferHeightPix ) )
  {
    int tgtTop = tgtTopOri;
    int tgtBottom = tgtBottomOri;
    int tgtLeft = tgtLeftOri;
    int tgtRight = tgtRightOri;

    // Deal with situations that requires adjusting tgt coordinates.
    // This is due rounding in QgsRasterBlock::subRect() and
    // when the difference between the raster extent and the
    // request extent is less than half of the request resolution.
    // This is to avoid to send dfXOff, dfYOff, dfXSize, dfYSize values that
    // would be outside of the raster extent, as GDAL (at least currently)
    // rejects them
    if ( reqExtent.xMinimum() + tgtLeft * reqXRes < mExtent.xMinimum() )
    {
      if ( GDALRasterIO( gdalBand, GF_Read, 0, srcTop, 1, srcHeight,
                         static_cast<char *>( data ) + tgtTopOri * bufferWidthPix * dataSize,
                         1, tgtBottomOri - tgtTopOri + 1, type,
                         dataSize, dataSize * bufferWidthPix ) != CE_None )
      {
        return false;
      }
      tgtLeft ++;
    }
    if ( reqExtent.yMaximum() - tgtTop * reqYRes > mExtent.yMaximum() )
    {
      if ( GDALRasterIO( gdalBand, GF_Read, srcLeft, 0, srcWidth, 1,
                         static_cast<char *>( data ) + tgtLeftOri * dataSize,
                         tgtRightOri - tgtLeftOri + 1, 1, type,
                         dataSize, dataSize * bufferWidthPix ) != CE_None )
      {
        return false;
      }
      tgtTop ++;
    }
    if ( reqExtent.xMinimum() + ( tgtRight + 1 ) * reqXRes > mExtent.xMaximum() )
    {
      if ( GDALRasterIO( gdalBand, GF_Read, xSize() - 1, srcTop, 1, srcHeight,
                         static_cast<char *>( data ) + ( tgtTopOri * bufferWidthPix + tgtRightOri ) * dataSize,
                         1, tgtBottomOri - tgtTopOri + 1, type,
                         dataSize, dataSize * bufferWidthPix ) != CE_None )
      {
        return false;
      }
      tgtRight --;
    }
    if ( reqExtent.yMaximum() - ( tgtBottom + 1 ) * reqYRes < mExtent.yMinimum() )
    {
      if ( GDALRasterIO( gdalBand, GF_Read, srcLeft, ySize() - 1, srcWidth, 1,
                         static_cast<char *>( data ) + ( tgtBottomOri * bufferWidthPix + tgtLeftOri ) * dataSize,
                         tgtRightOri - tgtLeftOri + 1, 1, type,
                         dataSize, dataSize * bufferWidthPix ) != CE_None )
      {
        return false;
      }
      tgtBottom --;
    }

    int tgtWidth = tgtRight - tgtLeft + 1;
    int tgtHeight = tgtBottom - tgtTop + 1;
    if ( tgtWidth > 0 && tgtHeight > 0 )
    {
      QgsDebugMsgLevel( QStringLiteral( "using GDAL resampling path" ), 5 );
      GDALRasterIOExtraArg sExtraArg;
      INIT_RASTERIO_EXTRA_ARG( sExtraArg );

      ResamplingMethod method;
      if ( resamplingFactor < 1 )
      {
        method = mZoomedInResamplingMethod;
      }
      else if ( resamplingFactor < 1.1 )
      {
        // very close to nominal resolution ==> use either zoomed out resampler or zoomed in resampler
        if ( mZoomedOutResamplingMethod != ResamplingMethod::Nearest )
          method = mZoomedOutResamplingMethod;
        else
          method = mZoomedInResamplingMethod;
      }
      else
      {
        method = mZoomedOutResamplingMethod;
      }
      sExtraArg.eResampleAlg = getGDALResamplingAlg( method );

      if ( mMaskBandExposedAsAlpha &&
           bandNo == GDALGetRasterCount( mGdalDataset ) + 1 &&
           sExtraArg.eResampleAlg != GRIORA_NearestNeighbour &&
           sExtraArg.eResampleAlg != GRIORA_Bilinear )
      {
        // As time of writing in GDAL up to 3.1, there's a difference of behavior
        // when using non-nearest resampling on mask bands.
        // With bilinear, values are returned in [0,255] range
        // whereas with cubic (and other convolution-based methods), there are in [0,1] range.
        // As we want [0,255] range, fallback to Bilinear for the mask band
        sExtraArg.eResampleAlg = GRIORA_Bilinear;
      }

      sExtraArg.bFloatingPointWindowValidity = true;
      sExtraArg.dfXOff = ( reqExtent.xMinimum() + tgtLeft * reqXRes - mExtent.xMinimum() ) / srcXRes;
      sExtraArg.dfYOff = ( mExtent.yMaximum() - ( reqExtent.yMaximum() - tgtTop * reqYRes ) ) / -srcYRes;
      sExtraArg.dfXSize = tgtWidth * reqXRes / srcXRes;
      sExtraArg.dfYSize = tgtHeight * reqYRes / -srcYRes;
      return GDALRasterIOEx( gdalBand, GF_Read,
                             static_cast<int>( std::floor( sExtraArg.dfXOff ) ),
                             static_cast<int>( std::floor( sExtraArg.dfYOff ) ),
                             std::max( 1, static_cast<int>( std::floor( sExtraArg.dfXSize ) ) ),
                             std::max( 1, static_cast<int>( std::floor( sExtraArg.dfYSize ) ) ),
                             static_cast<char *>( data ) +
                             ( tgtTop * bufferWidthPix + tgtLeft ) * dataSize,
                             tgtWidth,
                             tgtHeight,
                             type,
                             dataSize,
                             dataSize * bufferWidthPix,
                             &sExtraArg ) == CE_None;
    }
  }
  // Provider resampling was asked but we cannot do it in a performant way
  // (too much downsampling compared to the allowed maximum resampling factor),
  // so fallback to something replicating QgsRasterResampleFilter behavior
  else if ( mProviderResamplingEnabled &&
            mZoomedOutResamplingMethod != QgsRasterDataProvider::ResamplingMethod::Nearest &&
            resamplingFactor > 1 )
  {
    // Do the resampling in two steps:
    // - downsample with nearest neighbour down to tgtWidth * mMaxOversampling, tgtHeight * mMaxOversampling
    // - then downsample with mZoomedOutResamplingMethod down to tgtWidth, tgtHeight
    const int tgtWidth = tgtRightOri - tgtLeftOri + 1;
    const int tgtHeight = tgtBottomOri - tgtTopOri + 1;

    const int tmpWidth = static_cast<int>( tgtWidth  * mMaxOversampling + 0.5 );
    const int tmpHeight = static_cast<int>( tgtHeight * mMaxOversampling + 0.5 );

    // Allocate temporary block
    size_t bufferSize = dataSize * static_cast<size_t>( tmpWidth ) * static_cast<size_t>( tmpHeight );
#ifdef Q_PROCESSOR_X86_32
    // Safety check for 32 bit systems
    qint64 _buffer_size = dataSize * static_cast<qint64>( tmpWidth ) * static_cast<qint64>( tmpHeight );
    if ( _buffer_size != static_cast<qint64>( bufferSize ) )
    {
      QgsDebugMsg( QStringLiteral( "Integer overflow calculating buffer size on a 32 bit system." ) );
      return false;
    }
#endif
    char *tmpBlock = static_cast<char *>( qgsMalloc( bufferSize ) );
    if ( ! tmpBlock )
    {
      QgsDebugMsgLevel( QStringLiteral( "Couldn't allocate temporary buffer of %1 bytes" ).arg( dataSize * tmpWidth * tmpHeight ), 5 );
      return false;
    }
    CPLErrorReset();


    CPLErr err = gdalRasterIO( gdalBand, GF_Read,
                               srcLeft, srcTop, srcWidth, srcHeight,
                               static_cast<void *>( tmpBlock ),
                               tmpWidth, tmpHeight, type,
                               0, 0, feedback );

    if ( err != CPLE_None )
    {
      const QString lastError = QString::fromUtf8( CPLGetLastErrorMsg() ) ;
      if ( feedback )
        feedback->appendError( lastError );

      QgsLogger::warning( "RasterIO error: " + lastError );
      qgsFree( tmpBlock );
      return false;
    }

    GDALDriverH hDriverMem = GDALGetDriverByName( "MEM" );
    if ( !hDriverMem )
    {
      qgsFree( tmpBlock );
      return false;
    }
    gdal::dataset_unique_ptr hSrcDS( GDALCreate(
                                       hDriverMem, "", tmpWidth, tmpHeight, 0, GDALGetRasterDataType( gdalBand ), nullptr ) );

    char **papszOptions = QgsGdalUtils::papszFromStringList( QStringList()
                          << QStringLiteral( "PIXELOFFSET=%1" ).arg( dataSize )
                          << QStringLiteral( "LINEOFFSET=%1" ).arg( dataSize * tmpWidth )
                          << QStringLiteral( "DATAPOINTER=%1" ).arg( reinterpret_cast< qulonglong >( tmpBlock ) ) );
    GDALAddBand( hSrcDS.get(),  GDALGetRasterDataType( gdalBand ), papszOptions );
    CSLDestroy( papszOptions );

    GDALRasterIOExtraArg sExtraArg;
    INIT_RASTERIO_EXTRA_ARG( sExtraArg );

    sExtraArg.eResampleAlg = getGDALResamplingAlg( mZoomedOutResamplingMethod );

    CPLErr eErr = GDALRasterIOEx( GDALGetRasterBand( hSrcDS.get(), 1 ),
                                  GF_Read,
                                  0, 0, tmpWidth, tmpHeight,
                                  static_cast<char *>( data ) +
                                  ( tgtTopOri * bufferWidthPix + tgtLeftOri ) * dataSize,
                                  tgtWidth,
                                  tgtHeight,
                                  type,
                                  dataSize,
                                  dataSize * bufferWidthPix,
                                  &sExtraArg );

    qgsFree( tmpBlock );

    return eErr == CE_None;
  }

  const int tgtTop = tgtTopOri;
  const int tgtBottom = tgtBottomOri;
  const int tgtLeft = tgtLeftOri;
  const int tgtRight = tgtRightOri;
  QgsDebugMsgLevel( QStringLiteral( "tgtTop = %1 tgtBottom = %2 tgtLeft = %3 tgtRight = %4" ).arg( tgtTop ).arg( tgtBottom ).arg( tgtLeft ).arg( tgtRight ), 5 );
  // target size in pizels
  const int tgtWidth = tgtRight - tgtLeft + 1;
  const int tgtHeight = tgtBottom - tgtTop + 1;

  QgsDebugMsgLevel( QStringLiteral( "srcTop = %1 srcBottom = %2 srcWidth = %3 srcHeight = %4" ).arg( srcTop ).arg( srcBottom ).arg( srcWidth ).arg( srcHeight ), 5 );

  // Determine the dimensions of the buffer into which we will ask GDAL to write
  // pixels.
  // In downsampling scenarios, we will use the request resolution to compute the dimension
  // In upsampling (or exactly at raster resolution) scenarios, we will use the raster resolution to compute the dimension
  int tmpWidth = srcWidth;
  int tmpHeight = srcHeight;

  if ( reqXRes > srcXRes )
  {
    // downsampling
    tmpWidth = static_cast<int>( std::round( srcWidth * srcXRes / reqXRes ) );
  }
  if ( reqYRes > std::fabs( srcYRes ) )
  {
    // downsampling
    tmpHeight = static_cast<int>( std::round( -1.*srcHeight * srcYRes / reqYRes ) );
  }

  const double tmpXMin = mExtent.xMinimum() + srcLeft * srcXRes;
  const double tmpYMax = mExtent.yMaximum() + srcTop * srcYRes;
  QgsDebugMsgLevel( QStringLiteral( "tmpXMin = %1 tmpYMax = %2 tmpWidth = %3 tmpHeight = %4" ).arg( tmpXMin ).arg( tmpYMax ).arg( tmpWidth ).arg( tmpHeight ), 5 );

  // Allocate temporary block
  size_t bufferSize = dataSize * static_cast<size_t>( tmpWidth ) * static_cast<size_t>( tmpHeight );
#ifdef Q_PROCESSOR_X86_32
  // Safety check for 32 bit systems
  qint64 _buffer_size = dataSize * static_cast<qint64>( tmpWidth ) * static_cast<qint64>( tmpHeight );
  if ( _buffer_size != static_cast<qint64>( bufferSize ) )
  {
    QgsDebugMsg( QStringLiteral( "Integer overflow calculating buffer size on a 32 bit system." ) );
    return false;
  }
#endif
  char *tmpBlock = static_cast<char *>( qgsMalloc( bufferSize ) );
  if ( ! tmpBlock )
  {
    QgsDebugMsgLevel( QStringLiteral( "Couldn't allocate temporary buffer of %1 bytes" ).arg( dataSize * tmpWidth * tmpHeight ), 5 );
    return false;
  }
  CPLErrorReset();

  CPLErr err = gdalRasterIO( gdalBand, GF_Read,
                             srcLeft, srcTop, srcWidth, srcHeight,
                             static_cast<void *>( tmpBlock ),
                             tmpWidth, tmpHeight, type,
                             0, 0, feedback );

  if ( err != CPLE_None )
  {
    const QString lastError = QString::fromUtf8( CPLGetLastErrorMsg() ) ;
    if ( feedback )
      feedback->appendError( lastError );

    QgsLogger::warning( "RasterIO error: " + lastError );
    qgsFree( tmpBlock );
    return false;
  }

  const double tmpXRes = srcWidth * srcXRes / tmpWidth;
  const double tmpYRes = srcHeight * srcYRes / tmpHeight; // negative

  double y = intersectExtent.yMaximum() - 0.5 * reqYRes;
  for ( int row = 0; row < tgtHeight; row++ )
  {
    int tmpRow = static_cast<int>( std::floor( -1. * ( tmpYMax - y ) / tmpYRes ) );
    tmpRow = std::min( tmpRow, tmpHeight - 1 );

    char *srcRowBlock = tmpBlock + dataSize * tmpRow * tmpWidth;
    char *dstRowBlock = ( char * )data + dataSize * ( tgtTop + row ) * bufferWidthPix;

    double x = ( intersectExtent.xMinimum() + 0.5 * reqXRes - tmpXMin ) / tmpXRes; // cell center
    double increment = reqXRes / tmpXRes;

    char *dst = dstRowBlock + dataSize * tgtLeft;
    char *src = srcRowBlock;
    int tmpCol = 0;
    int lastCol = 0;
    for ( int col = 0; col < tgtWidth; ++col )
    {
      // std::floor() is quite slow! Use just cast to int.
      tmpCol = static_cast<int>( x );
      tmpCol = std::min( tmpCol, tmpWidth - 1 );
      if ( tmpCol > lastCol )
      {
        src += ( tmpCol - lastCol ) * dataSize;
        lastCol = tmpCol;
      }
      memcpy( dst, src, dataSize );
      dst += dataSize;
      x += increment;
    }
    y -= reqYRes;
  }

  qgsFree( tmpBlock );
  return true;
}

/**
 * \param bandNumber the number of the band for which you want a color table
 * \param list a pointer the object that will hold the color table
 * \return TRUE if a color table was able to be read, FALSE otherwise
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

  if ( mDriverName == QLatin1String( "netCDF" ) || mDriverName == QLatin1String( "GTiff" ) )
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
          return tr( "Band" ) + QStringLiteral( " %1: %2" ).arg( bandNumber, 1 + ( int ) std::log10( ( float ) bandCount() ), 10, QChar( '0' ) ).arg( bandNameValues.join( QLatin1String( " / " ) ) );
        }
      }
    }
  }
  QString generatedBandName = QgsRasterDataProvider::generateBandName( bandNumber );
  GDALRasterBandH myGdalBand = getBand( bandNumber );
  if ( ! myGdalBand )
  {
    QgsLogger::warning( QStringLiteral( "Band %1 does not exist." ).arg( bandNumber ) );
    return QString();
  }

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
      if ( sourceDataType( i ) == Qgis::DataType::Float32 )
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
}

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

  double value{0};
  CPLErr err {CE_Failure};

  const GDALDataType dataType {GDALGetRasterDataType( hBand )};
  switch ( dataType )
  {
    case GDT_Byte:
    {
      unsigned char tempVal{0};
      err = GDALRasterIO( hBand, GF_Read, col, row, 1, 1,
                          &tempVal, 1, 1, dataType, 0, 0 );
      value = static_cast<double>( tempVal );
      break;
    }
    case GDT_UInt16:
    {
      uint16_t tempVal{0};
      err = GDALRasterIO( hBand, GF_Read, col, row, 1, 1,
                          &tempVal, 1, 1, dataType, 0, 0 );
      value = static_cast<double>( tempVal );
      break;
    }
    case GDT_Int16:
    {
      int16_t tempVal{0};
      err = GDALRasterIO( hBand, GF_Read, col, row, 1, 1,
                          &tempVal, 1, 1, dataType, 0, 0 );
      value = static_cast<double>( tempVal );
      break;
    }
    case GDT_UInt32:
    {
      uint32_t tempVal{0};
      err = GDALRasterIO( hBand, GF_Read, col, row, 1, 1,
                          &tempVal, 1, 1, dataType, 0, 0 );
      value = static_cast<double>( tempVal );
      break;
    }
    case GDT_Int32:
    {
      int32_t tempVal{0};
      err = GDALRasterIO( hBand, GF_Read, col, row, 1, 1,
                          &tempVal, 1, 1, dataType, 0, 0 );
      value = static_cast<double>( tempVal );
      break;
    }
    case GDT_Float32:
    {
      float tempVal{0};
      err = GDALRasterIO( hBand, GF_Read, col, row, 1, 1,
                          &tempVal, 1, 1, dataType, 0, 0 );
      value = static_cast<double>( tempVal );
      break;
    }
    case GDT_Float64:
    {
      // No need to cast for double
      err = GDALRasterIO( hBand, GF_Read, col, row, 1, 1,
                          &value, 1, 1, dataType, 0, 0 );
      break;
    }
    case GDT_CInt16:
    case GDT_CInt32:
    case GDT_CFloat32:
    case GDT_CFloat64:
      QgsDebugMsg( QStringLiteral( "Raster complex data type is not supported!" ) );
      break;
    case GDT_TypeCount:
      QgsDebugMsg( QStringLiteral( "Raster data type GDT_TypeCount is not supported!" ) );
      break;
    case GDT_Unknown:
      QgsDebugMsg( QStringLiteral( "Raster data type is unknown!" ) );
      break;
  }
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
                   | QgsRasterDataProvider::Remove
                   | QgsRasterDataProvider::Prefetch;
  if ( mDriverName != QLatin1String( "WMS" ) )
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
      case Qgis::DataType::UnknownDataType:
      case Qgis::DataType::ARGB32:
      case Qgis::DataType::ARGB32_Premultiplied:
        return myDataType;
      case Qgis::DataType::Byte:
      case Qgis::DataType::UInt16:
      case Qgis::DataType::Int16:
      case Qgis::DataType::UInt32:
      case Qgis::DataType::Int32:
      case Qgis::DataType::Float32:
      case Qgis::DataType::CInt16:
        myDataType = Qgis::DataType::Float32;
        break;
      case Qgis::DataType::Float64:
      case Qgis::DataType::CInt32:
      case Qgis::DataType::CFloat32:
        myDataType = Qgis::DataType::Float64;
        break;
      case Qgis::DataType::CFloat64:
        return myDataType;
    }
  }
  return myDataType;
}

Qgis::DataType QgsGdalProvider::dataType( int bandNo ) const
{
  if ( mMaskBandExposedAsAlpha && bandNo == mBandCount )
    return dataTypeFromGdal( GDT_Byte );

  if ( bandNo <= 0 || bandNo > mGdalDataType.count() ) return Qgis::DataType::UnknownDataType;

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
  QgsDebugMsgLevel( QStringLiteral( "valid = %1" ).arg( mValid ), 4 );
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

QString QgsGdalProvider::providerKey()
{
  return PROVIDER_KEY;
};

QString QgsGdalProvider::description() const
{
  return PROVIDER_DESCRIPTION;
}

QgsRasterDataProvider::ProviderCapabilities QgsGdalProvider::providerCapabilities() const
{
  return ProviderCapability::ProviderHintBenefitsFromResampling |
         ProviderCapability::ProviderHintCanPerformProviderResampling |
         ProviderCapability::ReloadData;
}

QList<QgsProviderSublayerDetails> QgsGdalProvider::sublayerDetails( GDALDatasetH dataset, const QString &baseUri )
{
  if ( !dataset )
  {
    QgsDebugMsg( QStringLiteral( "dataset is nullptr" ) );
    return {};
  }

  GDALDriverH hDriver = GDALGetDatasetDriver( dataset );
  const QString gdalDriverName = GDALGetDriverShortName( hDriver );

  QList<QgsProviderSublayerDetails> res;

  char **metadata = GDALGetMetadata( dataset, "SUBDATASETS" );

  QVariantMap uriParts = decodeGdalUri( baseUri );
  const QString datasetPath = uriParts.value( QStringLiteral( "path" ) ).toString();

  if ( metadata )
  {
    const int subdatasetCount = CSLCount( metadata ) / 2;
    for ( int i = 1; i <= subdatasetCount; i++ )
    {
      const char *name = CSLFetchNameValue( metadata, CPLSPrintf( "SUBDATASET_%d_NAME", i ) );
      const char *desc = CSLFetchNameValue( metadata, CPLSPrintf( "SUBDATASET_%d_DESC", i ) );
      if ( name && desc )
      {
        QString layerName = QString::fromUtf8( name );
        QString uri = layerName;
        QString layerDesc = QString::fromUtf8( desc );

        // For GeoPackage, desc is often "table - identifier" where table=identifier
        // In that case, just keep one.
        int sepIdx = layerDesc.indexOf( QLatin1String( " - " ) );
        if ( sepIdx > 0 )
        {
          layerName = layerDesc.left( sepIdx );
          layerDesc = layerDesc.mid( sepIdx + 3 );
        }
        else
        {
          // try to extract layer name from a path like 'NETCDF:"/baseUri":cell_node'
          sepIdx = layerName.indexOf( datasetPath + "\":" );
          if ( sepIdx >= 0 )
          {
            layerName = layerName.mid( layerName.indexOf( datasetPath + "\":" ) + datasetPath.length() + 2 );
          }
        }

        QgsProviderSublayerDetails details;
        details.setProviderKey( PROVIDER_KEY );
        details.setType( QgsMapLayerType::RasterLayer );
        details.setName( layerName );
        details.setDescription( layerDesc );
        details.setLayerNumber( i );
        details.setDriverName( gdalDriverName );

        const QVariantMap layerUriParts = decodeGdalUri( uri );
        // update original uri parts with this layername and path -- this ensures that other uri components
        // like open options are preserved for the sublayer uris
        uriParts.insert( QStringLiteral( "layerName" ), layerUriParts.value( QStringLiteral( "layerName" ) ) );
        uriParts.insert( QStringLiteral( "path" ), layerUriParts.value( QStringLiteral( "path" ) ) );
        details.setUri( encodeGdalUri( uriParts ) );

        res << details;
      }
    }
  }

  return res;
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

  QgsDebugMsgLevel( QStringLiteral( "theBandNo = %1 binCount = %2 minimum = %3 maximum = %4 sampleSize = %5" ).arg( bandNo ).arg( binCount ).arg( minimum ).arg( maximum ).arg( sampleSize ), 3 );

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
    QgsDebugMsgLevel( QStringLiteral( "Not supported by GDAL." ), 2 );
    return false;
  }

  if ( ( sourceHasNoDataValue( bandNo ) && !useSourceNoDataValue( bandNo ) ) ||
       !userNoDataValues( bandNo ).isEmpty() )
  {
    QgsDebugMsgLevel( QStringLiteral( "Custom no data values -> GDAL histogram not sufficient." ), 3 );
    return false;
  }

  QgsDebugMsgLevel( QStringLiteral( "Looking for GDAL histogram" ), 4 );

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
    QgsDebugMsgLevel( QStringLiteral( "Cannot get default GDAL histogram" ), 2 );
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
    QgsDebugMsgLevel( QStringLiteral( "Params do not match binCount: %1 x %2, minVal: %3 x %4, maxVal: %5 x %6" ).arg( myBinCount ).arg( myHistogram.binCount ).arg( myMinVal ).arg( myExpectedMinVal ).arg( myMaxVal ).arg( myExpectedMaxVal ), 2 );
    return false;
  }

  QgsDebugMsgLevel( QStringLiteral( "GDAL has cached histogram" ), 2 );

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

  QgsDebugMsgLevel( QStringLiteral( "theBandNo = %1 binCount = %2 minimum = %3 maximum = %4 sampleSize = %5" ).arg( bandNo ).arg( binCount ).arg( minimum ).arg( maximum ).arg( sampleSize ), 2 );

  QgsRasterHistogram myHistogram;
  initHistogram( myHistogram, bandNo, binCount, minimum, maximum, boundingBox, sampleSize, includeOutOfRange );

  // Find cached
  const auto constMHistograms = mHistograms;
  for ( const QgsRasterHistogram &histogram : constMHistograms )
  {
    if ( histogram == myHistogram )
    {
      QgsDebugMsgLevel( QStringLiteral( "Using cached histogram." ), 2 );
      return histogram;
    }
  }

  if ( ( sourceHasNoDataValue( bandNo ) && !useSourceNoDataValue( bandNo ) ) ||
       !userNoDataValues( bandNo ).isEmpty() )
  {
    QgsDebugMsgLevel( QStringLiteral( "Custom no data values, using generic histogram." ), 2 );
    return QgsRasterDataProvider::histogram( bandNo, binCount, minimum, maximum, boundingBox, sampleSize, includeOutOfRange, feedback );
  }

  if ( myHistogram.extent != extent() )
  {
    QgsDebugMsgLevel( QStringLiteral( "Not full extent, using generic histogram." ), 2 );
    return QgsRasterDataProvider::histogram( bandNo, binCount, minimum, maximum, boundingBox, sampleSize, includeOutOfRange, feedback );
  }

  QgsDebugMsgLevel( QStringLiteral( "Computing GDAL histogram" ), 2 );

  GDALRasterBandH myGdalBand = getBand( bandNo );

  int bApproxOK = false;
  if ( sampleSize > 0 )
  {
    // cast to double, integer could overflow
    if ( ( static_cast<double>( xSize() ) * static_cast<double>( ySize() ) / sampleSize ) > 2 ) // not perfect
    {
      QgsDebugMsgLevel( QStringLiteral( "Approx" ), 2 );
      bApproxOK = true;
    }
  }

  QgsDebugMsgLevel( QStringLiteral( "xSize() = %1 ySize() = %2 sampleSize = %3 bApproxOK = %4" ).arg( xSize() ).arg( ySize() ).arg( sampleSize ).arg( bApproxOK ), 2 );

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
    QgsDebugMsgLevel( QStringLiteral( "Cannot get histogram" ), 2 );
    delete [] myHistogramArray;
    return myHistogram;
  }

#endif

  for ( int myBin = 0; myBin < myHistogram.binCount; myBin++ )
  {
    myHistogram.histogramVector.push_back( myHistogramArray[myBin] );
    myHistogram.nonNullCount += myHistogramArray[myBin];
  }

  myHistogram.valid = true;

  delete [] myHistogramArray;

  QgsDebugMsgLevel( ">>>>> Histogram vector now contains " + QString::number( myHistogram.histogramVector.size() ) + " elements", 3 );

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

  //TODO: Consider making rasterPyramidList modifiable by this method to indicate if the pyramid exists after build attempt
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

      mGdalBaseDataset = gdalOpen( dataSourceUri( true ), GDAL_OF_UPDATE );

      // if the dataset couldn't be opened in read / write mode, tell the user
      if ( !mGdalBaseDataset )
      {
        mGdalBaseDataset = gdalOpen( dataSourceUri( true ), GDAL_OF_READONLY );
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
    const auto constConfigOptions = configOptions;
    for ( const QString &option : constConfigOptions )
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
        QgsDebugMsgLevel( QStringLiteral( "set option %1=%2" ).arg( key.data(), value.data() ), 2 );
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
    QgsDebugMsgLevel( QStringLiteral( "Build pyramids:: Level %1" ).arg( myRasterPyramidIterator->getLevel() ), 2 );
    QgsDebugMsgLevel( QStringLiteral( "x:%1" ).arg( myRasterPyramidIterator->getXDim() ), 2 );
    QgsDebugMsgLevel( QStringLiteral( "y:%1" ).arg( myRasterPyramidIterator->getYDim() ), 2 );
    QgsDebugMsgLevel( QStringLiteral( "exists : %1" ).arg( myRasterPyramidIterator->getExists() ), 2 );
#endif
    if ( myRasterPyramidIterator->getBuild() )
    {
      QgsDebugMsgLevel( QStringLiteral( "adding overview at level %1 to list"
                                      ).arg( myRasterPyramidIterator->getLevel() ), 2 );
      myOverviewLevelsVector.append( myRasterPyramidIterator->getLevel() );
    }
  }
  /* From : http://www.gdal.org/classGDALDataset.html#a2aa6f88b3bbc840a5696236af11dde15
   * pszResampling : one of "NEAREST", "GAUSS", "CUBIC", "CUBICSPLINE" (GDAL >= 2.0), "LANCZOS" ( GDAL >= 2.0),
   * "BILINEAR" ( GDAL >= 2.0), "AVERAGE", "MODE" or "NONE" controlling the downsampling method applied.
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
  QgsDebugMsgLevel( QStringLiteral( "Building overviews at %1 levels using resampling method %2"
                                  ).arg( myOverviewLevelsVector.size() ).arg( method ), 2 );
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
      mGdalBaseDataset = gdalOpen( dataSourceUri( true ), mUpdate ? GDAL_OF_UPDATE : GDAL_OF_READONLY );
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
      QgsDebugMsgLevel( QStringLiteral( "Building pyramids finished OK" ), 2 );
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

  QgsDebugMsgLevel( QStringLiteral( "Pyramid overviews built" ), 2 );

  // Observed problem: if a *.rrd file exists and GDALBuildOverviews() is called,
  // the *.rrd is deleted and no overviews are created, if GDALBuildOverviews()
  // is called next time, it crashes somewhere in GDAL:
  // https://trac.osgeo.org/gdal/ticket/4831
  // Crash can be avoided if dataset is reopened, fixed in GDAL 1.9.2
  if ( format == QgsRaster::PyramidsInternal )
  {
    QgsDebugMsgLevel( QStringLiteral( "Reopening dataset ..." ), 2 );
    //close the gdal dataset and reopen it in read only mode
    GDALClose( mGdalBaseDataset );
    mGdalBaseDataset = gdalOpen( dataSourceUri( true ), mUpdate ? GDAL_OF_UPDATE : GDAL_OF_READONLY );
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

QList<QgsRasterPyramid> QgsGdalProvider::buildPyramidList( const QList<int> &list )
{
  QList< int > overviewList = list;
  QMutexLocker locker( mpMutex );

  int myWidth = mWidth;
  int myHeight = mHeight;
  GDALRasterBandH myGDALBand = GDALGetRasterBand( mGdalDataset, 1 ); //just use the first band

  mPyramidList.clear();

  // if overviewList is empty (default) build the pyramid list
  if ( overviewList.isEmpty() )
  {
    int myDivisor = 2;

    QgsDebugMsgLevel( QStringLiteral( "Building initial pyramid list" ), 2 );

    while ( ( myWidth / myDivisor > 32 ) && ( ( myHeight / myDivisor ) > 32 ) )
    {
      overviewList.append( myDivisor );
      //sqare the divisor each step
      myDivisor = ( myDivisor * 2 );
    }
  }

  // loop over pyramid list
  const auto constOverviewList = overviewList;
  for ( int myDivisor : constOverviewList )
  {
    //
    // First we build up a list of potential pyramid layers
    //

    QgsRasterPyramid myRasterPyramid;
    myRasterPyramid.setLevel( myDivisor );
    myRasterPyramid.setXDim( ( int )( 0.5 + ( myWidth / static_cast<double>( myDivisor ) ) ) ); // NOLINT
    myRasterPyramid.setYDim( ( int )( 0.5 + ( myHeight / static_cast<double>( myDivisor ) ) ) ); // NOLINT
    myRasterPyramid.setExists( false );

    QgsDebugMsgLevel( QStringLiteral( "Pyramid %1 xDim %2 yDim %3" ).arg( myRasterPyramid.getLevel() ).arg( myRasterPyramid.getXDim() ).arg( myRasterPyramid.getYDim() ), 2 );

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
        QgsDebugMsgLevel( "Checking whether " + QString::number( myRasterPyramid.getXDim() ) + " x " +
                          QString::number( myRasterPyramid.getYDim() ) + " matches " +
                          QString::number( myOverviewXDim ) + " x " + QString::number( myOverviewYDim ), 2 );


        if ( ( myOverviewXDim <= ( myRasterPyramid.getXDim() + myNearMatchLimit ) ) &&
             ( myOverviewXDim >= ( myRasterPyramid.getXDim() - myNearMatchLimit ) ) &&
             ( myOverviewYDim <= ( myRasterPyramid.getYDim() + myNearMatchLimit ) ) &&
             ( myOverviewYDim >= ( myRasterPyramid.getYDim() - myNearMatchLimit ) ) )
        {
          //right we have a match so adjust the a / y before they get added to the list
          myRasterPyramid.setXDim( myOverviewXDim );
          myRasterPyramid.setYDim( myOverviewYDim );
          myRasterPyramid.setExists( true );
          QgsDebugMsgLevel( QStringLiteral( ".....YES!" ), 2 );
        }
        else
        {
          //no match
          QgsDebugMsgLevel( QStringLiteral( ".....no." ), 2 );
        }
      }
    }
    mPyramidList.append( myRasterPyramid );
  }

  return mPyramidList;
}

QStringList QgsGdalProvider::subLayers() const
{
  QStringList res;
  res.reserve( mSubLayers.size() );
  for ( const QgsProviderSublayerDetails &layer : mSubLayers )
    res << layer.uri() + QgsDataProvider::sublayerSeparator() + layer.description();
  return res;
}

QVariantMap QgsGdalProviderMetadata::decodeUri( const QString &uri ) const
{
  return QgsGdalProviderBase::decodeGdalUri( uri );
}

QString QgsGdalProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  return  QgsGdalProviderBase::encodeGdalUri( parts );
}

bool QgsGdalProviderMetadata::uriIsBlocklisted( const QString &uri ) const
{
  const QVariantMap parts = decodeUri( uri );
  if ( !parts.contains( QStringLiteral( "path" ) ) )
    return false;

  QFileInfo fi( parts.value( QStringLiteral( "path" ) ).toString() );
  const QString suffix = fi.completeSuffix();

  // internal details only
  if ( suffix.compare( QLatin1String( "aux.xml" ), Qt::CaseInsensitive ) == 0 || suffix.endsWith( QLatin1String( ".aux.xml" ), Qt::CaseInsensitive ) )
    return true;
  if ( suffix.compare( QLatin1String( "tif.xml" ), Qt::CaseInsensitive ) == 0 )
    return true;

  return false;
}


QgsGdalProvider *QgsGdalProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  Q_UNUSED( flags );
  return new QgsGdalProvider( uri, options );
}

/**
 * Convenience function for readily creating file filters.
 *
 * Given a long name for a file filter and a regular expression, return
 * a file filter string suitable for use in a QFileDialog::OpenFiles()
 * call.  The regular express, glob, will have both all lower and upper
 * case versions added.
 *
 * \note Copied from qgisapp.cpp.=
 * \todo XXX This should probably be generalized and moved to a standard
 * utility type thingy.
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

  QgsDebugMsgLevel( QStringLiteral( "GDAL driver count: %1" ).arg( GDALGetDriverCount() ), 2 );

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
    myGdalDriverLongName.remove( QRegularExpression( "\\(.*\\)$" ) );

    // if we have both the file name extension and the long name,
    // then we've all the information we need for the current
    // driver; therefore emit a file filter string and move to
    // the next driver
    if ( !( myGdalDriverExtensions.isEmpty() || myGdalDriverLongName.isEmpty() ) )
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
      const QStringList splitExtensions = myGdalDriverExtensions.split( ' ', QString::SkipEmptyParts );
#else
      const QStringList splitExtensions = myGdalDriverExtensions.split( ' ', Qt::SkipEmptyParts );
#endif

      // XXX add check for SDTS; in that case we want (*CATD.DDF)
      QString glob;

      for ( const QString &ext : splitExtensions )
      {
        // This hacking around that removes '/' is no longer necessary with GDAL 2.3
        extensions << QString( ext ).remove( '/' ).remove( '*' ).remove( '.' );
        if ( !glob.isEmpty() )
          glob += QLatin1Char( ' ' );
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
      else
      {
        catchallFilter << QString( GDALGetDescription( myGdalDriver ) );
      }
    } // each loaded GDAL driver

  }                           // each loaded GDAL driver

  // sort file filters alphabetically
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  QStringList filters = fileFiltersString.split( QStringLiteral( ";;" ), QString::SkipEmptyParts );
#else
  QStringList filters = fileFiltersString.split( QStringLiteral( ";;" ), Qt::SkipEmptyParts );
#endif
  filters.sort();
  fileFiltersString = filters.join( QLatin1String( ";;" ) ) + ";;";

  // VSIFileHandler (see qgsogrprovider.cpp) - second
  QgsSettings settings;
  if ( settings.value( QStringLiteral( "qgis/scanZipInBrowser2" ), "basic" ).toString() != QLatin1String( "no" ) )
  {
    fileFiltersString.prepend( createFileFilter_( QObject::tr( "GDAL/OGR VSIFileHandler" ), QStringLiteral( "*.zip *.gz *.tar *.tar.gz *.tgz" ) ) );
    extensions << QStringLiteral( "zip" ) << QStringLiteral( "gz" ) << QStringLiteral( "tar" ) << QStringLiteral( "tar.gz" ) << QStringLiteral( "tgz" );
  }

  // can't forget the all supported case
  QStringList exts;
  for ( const QString &ext : std::as_const( extensions ) )
    exts << QStringLiteral( "*.%1 *.%2" ).arg( ext, ext.toUpper() );
  fileFiltersString.prepend( QObject::tr( "All supported files" ) + QStringLiteral( " (%1);;" ).arg( exts.join( QLatin1Char( ' ' ) ) ) );

  // can't forget the default case - first
  fileFiltersString.prepend( QObject::tr( "All files" ) + " (*);;" );

  // cleanup
  if ( fileFiltersString.endsWith( QLatin1String( ";;" ) ) ) fileFiltersString.chop( 2 );

  QgsDebugMsgLevel( "Raster filter list built: " + fileFiltersString, 2 );
  QgsDebugMsgLevel( "Raster extension list built: " + extensions.join( ' ' ), 2 );
}                               // buildSupportedRasterFileFilter_()


bool QgsGdalProvider::isValidRasterFileName( QString const &fileNameQString, QString &retErrMsg )
{
  gdal::dataset_unique_ptr myDataset;

  QgsGdalProviderBase::registerGdalDrivers();

  CPLErrorReset();

  QString fileName = QgsGdalProvider::expandAuthConfig( fileNameQString );

  // Try to open using VSIFileHandler (see qgsogrprovider.cpp)
  // TODO suppress error messages and report in debug, like in OGR provider
  QString vsiPrefix = QgsZipItem::vsiPrefix( fileName );
  if ( !vsiPrefix.isEmpty() )
  {
    if ( !fileName.startsWith( vsiPrefix ) )
      fileName = vsiPrefix + fileName;
    QgsDebugMsgLevel( QStringLiteral( "Trying %1 syntax, fileName= %2" ).arg( vsiPrefix, fileName ), 2 );
  }

  //open the file using gdal making sure we have handled locale properly
  //myDataset = GDALOpen( QFile::encodeName( fileNameQString ).constData(), GA_ReadOnly );
  myDataset.reset( QgsGdalProviderBase::gdalOpen( fileName, GDAL_OF_READONLY ) );
  if ( !myDataset )
  {
    if ( CPLGetLastErrorNo() != CPLE_OpenFailed )
      retErrMsg = QString::fromUtf8( CPLGetLastErrorMsg() );
    return false;
  }
  else if ( GDALGetRasterCount( myDataset.get() ) == 0 )
  {
    if ( QgsGdalProvider::sublayerDetails( myDataset.get(), fileName ).isEmpty() )
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

  QgsDebugMsgLevel( QStringLiteral( "theBandNo = %1 sampleSize = %2" ).arg( bandNo ).arg( sampleSize ), 2 );

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
    QgsDebugMsgLevel( QStringLiteral( "Custom no data values -> GDAL statistics not sufficient." ), 2 );
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

  QgsDebugMsgLevel( QStringLiteral( "Looking for GDAL statistics" ), 2 );

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

  CPLErr myerval = GDALGetRasterStatistics( myGdalBand, bApproxOK, true, pdfMin, pdfMax, pdfMean, pdfStdDev );

  if ( CE_None == myerval ) // CE_Warning if cached not found
  {
    QgsDebugMsgLevel( QStringLiteral( "GDAL has cached statistics" ), 2 );
    return true;
  }

  return false;
}

QgsRasterBandStats QgsGdalProvider::bandStatistics( int bandNo, int stats, const QgsRectangle &boundingBox, int sampleSize, QgsRasterBlockFeedback *feedback )
{
  QMutexLocker locker( mpMutex );
  if ( !initIfNeeded() )
    return QgsRasterBandStats();

  QgsDebugMsgLevel( QStringLiteral( "theBandNo = %1 sampleSize = %2" ).arg( bandNo ).arg( sampleSize ), 2 );

  // TODO: null values set on raster layer!!!

  // Currently there is no API in GDAL to collect statistics of specified extent
  // or with defined sample size. We check first if we have cached stats, if not,
  // and it is not possible to use GDAL we call generic provider method,
  // otherwise we use GDAL (faster, cache)

  QgsRasterBandStats myRasterBandStats;
  initStatistics( myRasterBandStats, bandNo, stats, boundingBox, sampleSize );

  const auto constMStatistics = mStatistics;
  for ( const QgsRasterBandStats &stats : constMStatistics )
  {
    if ( stats.contains( myRasterBandStats ) )
    {
      QgsDebugMsgLevel( QStringLiteral( "Using cached statistics." ), 2 );
      return stats;
    }
  }

  // We cannot use GDAL stats if user disabled src no data value or set
  // custom  no data values
  if ( ( sourceHasNoDataValue( bandNo ) && !useSourceNoDataValue( bandNo ) ) ||
       !userNoDataValues( bandNo ).isEmpty() )
  {
    QgsDebugMsgLevel( QStringLiteral( "Custom no data values, using generic statistics." ), 2 );
    return QgsRasterDataProvider::bandStatistics( bandNo, stats, boundingBox, sampleSize, feedback );
  }

  int supportedStats = QgsRasterBandStats::Min | QgsRasterBandStats::Max
                       | QgsRasterBandStats::Range | QgsRasterBandStats::Mean
                       | QgsRasterBandStats::StdDev;

  QgsDebugMsgLevel( QStringLiteral( "theStats = %1 supportedStats = %2" ).arg( stats, 0, 2 ).arg( supportedStats, 0, 2 ), 2 );

  if ( myRasterBandStats.extent != extent() ||
       ( stats & ( ~supportedStats ) ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "Statistics not supported by provider, using generic statistics." ), 2 );
    return QgsRasterDataProvider::bandStatistics( bandNo, stats, boundingBox, sampleSize, feedback );
  }

  QgsDebugMsgLevel( QStringLiteral( "Using GDAL statistics." ), 2 );
  GDALRasterBandH myGdalBand = getBand( bandNo );

  //int bApproxOK = false; //as we asked for stats, don't get approx values
  // GDAL does not have sample size parameter in API, just bApproxOK or not,
  // we decide if approximation should be used according to
  // total size / sample size ratio
  int bApproxOK = false;
  if ( sampleSize > 0 )
  {
    if ( ( static_cast<double>( xSize() ) * static_cast<double>( ySize() ) / sampleSize ) > 2 ) // not perfect
    {
      bApproxOK = true;
    }
  }

  QgsDebugMsgLevel( QStringLiteral( "bApproxOK = %1" ).arg( bApproxOK ), 2 );

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

  QgsDebugMsgLevel( QStringLiteral( "myerval = %1" ).arg( myerval ), 2 );

  // if cached stats are not found, compute them
  if ( !bApproxOK || CE_None != myerval )
  {
    QgsDebugMsgLevel( QStringLiteral( "Calculating statistics by GDAL" ), 2 );
    myerval = GDALComputeRasterStatistics( myGdalBand, bApproxOK,
                                           &pdfMin, &pdfMax, &pdfMean, &pdfStdDev,
                                           progressCallback, &myProg );
    mStatisticsAreReliable = true;
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "Using GDAL cached statistics" ), 2 );
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
    QgsDebugMsgLevel( QStringLiteral( "************ STATS **************" ), 2 );
    QgsDebugMsgLevel( QStringLiteral( "MIN %1" ).arg( myRasterBandStats.minimumValue ), 2 );
    QgsDebugMsgLevel( QStringLiteral( "MAX %1" ).arg( myRasterBandStats.maximumValue ), 2 );
    QgsDebugMsgLevel( QStringLiteral( "RANGE %1" ).arg( myRasterBandStats.range ), 2 );
    QgsDebugMsgLevel( QStringLiteral( "MEAN %1" ).arg( myRasterBandStats.mean ), 2 );
    QgsDebugMsgLevel( QStringLiteral( "STDDEV %1" ).arg( myRasterBandStats.stdDev ), 2 );
#endif
  }
  else
  {
    myRasterBandStats.statsGathered = QgsRasterBandStats::Stats::None;
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
    QgsDebugMsgLevel( QStringLiteral( "Trying %1 syntax, uri= %2" ).arg( vsiPrefix, dataSourceUri() ), 2 );
  }

  gdalUri = dataSourceUri( true );

  CPLErrorReset();
  mGdalBaseDataset = gdalOpen( gdalUri, mUpdate ? GDAL_OF_UPDATE : GDAL_OF_READONLY );

  if ( !mGdalBaseDataset )
  {
    QString msg = QStringLiteral( "Cannot open GDAL dataset %1:\n%2" ).arg( dataSourceUri(), QString::fromUtf8( CPLGetLastErrorMsg() ) );
    appendError( ERRMSG( msg ) );
    return false;
  }

  QgsDebugMsgLevel( QStringLiteral( "GdalDataset opened" ), 2 );

  initBaseDataset();
  return mValid;
}


void QgsGdalProvider::initBaseDataset()
{
  mDriverName = GDALGetDriverShortName( GDALGetDatasetDriver( mGdalBaseDataset ) );
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

    gdal::warp_options_unique_ptr psWarpOptions( GDALCreateWarpOptions() );
    // Add alpha band to the output VRT dataset if there's no way for GDAL to
    // respresent the fact that some pixels should be transparent (in the empty regions
    // when the raster is rotated). For example, this fixes the issue for RGB rasters
    // (with no alpha channel) or single-band raster without "no data" value set.
    if ( GDALGetMaskFlags( GDALGetRasterBand( mGdalBaseDataset, 1 ) ) == GMF_ALL_VALID )
    {
      psWarpOptions->nDstAlphaBand = GDALGetRasterCount( mGdalBaseDataset ) + 1;
    }

    if ( GDALGetMetadata( mGdalBaseDataset, "RPC" ) )
    {
      mGdalDataset =
        QgsGdalUtils::rpcAwareAutoCreateWarpedVrt( mGdalBaseDataset, nullptr, nullptr,
            GRA_NearestNeighbour, 0.2, psWarpOptions.get() );
      mGdalTransformerArg = QgsGdalUtils::rpcAwareCreateTransformer( mGdalBaseDataset );
    }
    else
    {
      mGdalDataset =
        GDALAutoCreateWarpedVRT( mGdalBaseDataset, nullptr, nullptr,
                                 GRA_NearestNeighbour, 0.2, psWarpOptions.get() );
    }

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

  if ( !mGdalTransformerArg )
  {
    mGdalTransformerArg = GDALCreateGenImgProjTransformer( mGdalBaseDataset, nullptr, nullptr, nullptr, TRUE, 1.0, 0 );
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
  mSubLayers = QgsGdalProvider::sublayerDetails( mGdalDataset,  dataSourceUri() );

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
  QString crsWkt;
  if ( OGRSpatialReferenceH spatialRefSys = GDALGetSpatialRef( mGdalDataset ) )
  {
    crsWkt = QgsOgrUtils::OGRSpatialReferenceToWkt( spatialRefSys );
  }
  if ( crsWkt.isEmpty() )
  {
    if ( OGRSpatialReferenceH spatialRefSys = GDALGetGCPSpatialRef( mGdalDataset ) )
    {
      crsWkt = QgsOgrUtils::OGRSpatialReferenceToWkt( spatialRefSys );
    }
  }
  if ( !crsWkt.isEmpty() )
  {
    mCrs = QgsCoordinateReferenceSystem::fromWkt( crsWkt );
  }
  else
  {
    if ( mGdalBaseDataset != mGdalDataset &&
         GDALGetMetadata( mGdalBaseDataset, "RPC" ) )
    {
      // Warped VRT of RPC is in EPSG:4326
      mCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( QStringLiteral( "EPSG:4326" ) );
    }
    else
    {
      QgsDebugMsgLevel( QStringLiteral( "No valid CRS identified" ), 2 );
    }
  }

  //set up the coordinat transform - in the case of raster this is mainly used to convert
  //the inverese projection of the map extents of the canvas when zooming in etc. so
  //that they match the coordinate system of this layer

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
      QgsDebugMsgLevel( QStringLiteral( "GDALGetRasterNoDataValue = %1 is not representable in data type, so ignoring it" ).arg( myNoDataValue ), 2 );
      isValid = false;
    }
    if ( isValid )
    {
      QgsDebugMsgLevel( QStringLiteral( "GDALGetRasterNoDataValue = %1" ).arg( myNoDataValue ), 2 );
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
    // (for reprojection for example) -> always internally represent as wider type
    // with mInternalNoDataValue in reserve.
    // Not used
#if 0
    int myInternalGdalDataType = myGdalDataType;
    double myInternalNoDataValue = 123;
    switch ( srcDataType( i ) )
    {
      case Qgis::DataType::Byte:
        myInternalNoDataValue = -32768.0;
        myInternalGdalDataType = GDT_Int16;
        break;
      case Qgis::DataType::Int16:
        myInternalNoDataValue = -2147483648.0;
        myInternalGdalDataType = GDT_Int32;
        break;
      case Qgis::DataType::UInt16:
        myInternalNoDataValue = -2147483648.0;
        myInternalGdalDataType = GDT_Int32;
        break;
      case Qgis::DataType::Int32:
        // We believe that such values is no used in real data
        myInternalNoDataValue = -2147483648.0;
        break;
      case Qgis::DataType::UInt32:
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
  }

  if ( mMaskBandExposedAsAlpha )
  {
    mSrcNoDataValue.append( std::numeric_limits<double>::quiet_NaN() );
    mSrcHasNoDataValue.append( false );
    mUseSrcNoDataValue.append( false );
    mGdalDataType.append( GDT_Byte );
  }
}

QgsGdalProvider *QgsGdalProviderMetadata::createRasterDataProvider(
  const QString &uri,
  const QString &format,
  int nBands,
  Qgis::DataType type,
  int width, int height,
  double *geoTransform,
  const QgsCoordinateReferenceSystem &crs,
  const QStringList &createOptions )
{
  //get driver
  GDALDriverH driver = GDALGetDriverByName( format.toLocal8Bit().data() );
  if ( !driver )
  {
    QgsError error( "Cannot load GDAL driver " + format, QStringLiteral( "GDAL provider" ) );
    return new QgsGdalProvider( uri, error );
  }

  QgsDebugMsgLevel( "create options: " + createOptions.join( " " ), 2 );

  //create dataset
  CPLErrorReset();
  char **papszOptions = QgsGdalUtils::papszFromStringList( createOptions );
  gdal::dataset_unique_ptr dataset( GDALCreate( driver, uri.toUtf8().constData(), width, height, nBands, ( GDALDataType )type, papszOptions ) );
  CSLDestroy( papszOptions );
  if ( !dataset )
  {
    QgsError error( QStringLiteral( "Cannot create new dataset  %1:\n%2" ).arg( uri, QString::fromUtf8( CPLGetLastErrorMsg() ) ), QStringLiteral( "GDAL provider" ) );
    QgsDebugMsg( error.summary() );
    return new QgsGdalProvider( uri, error );
  }

  GDALSetGeoTransform( dataset.get(), geoTransform );
  GDALSetProjection( dataset.get(), crs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED_GDAL ).toLocal8Bit().data() );

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
    QgsDebugMsgLevel( QStringLiteral( "Waiting for ref counter for %1 to drop to 1" ).arg( dataSourceUri() ), 2 );
    QThread::msleep( 100 );
  }

  if ( mGdalDataset )
  {
    GDALDriverH driver = GDALGetDriverByName( mDriverName.toLocal8Bit().constData() );
    if ( !driver )
      return false;

    closeDataset();

    CPLErrorReset();
    CPLErr err = GDALDeleteDataset( driver, dataSourceUri( true ).toUtf8().constData() );
    if ( err != CPLE_None )
    {
      QgsLogger::warning( "RasterIO error: " + QString::fromUtf8( CPLGetLastErrorMsg() ) );
      QgsDebugMsg( "RasterIO error: " + QString::fromUtf8( CPLGetLastErrorMsg() ) );
      return false;
    }
    QgsDebugMsgLevel( QStringLiteral( "Raster dataset dataSourceUri() successfully deleted" ), 2 );
    return true;
  }
  return false;
}

/**
 * Builds the list of file filter strings to later be used by
 * QgisApp::addRasterLayer()
 *
 * We query GDAL for a list of supported raster formats; we then build
 * a list of file filter strings from that list.  We return a string
 * that contains this list that is suitable for use in a
 * QFileDialog::getOpenFileNames() call.
*/
QString QgsGdalProviderMetadata::filters( FilterType type )
{
  switch ( type )
  {
    case QgsProviderMetadata::FilterType::FilterRaster:
    {
      QString fileFiltersString;
      QStringList exts;
      QStringList wildcards;
      buildSupportedRasterFileFilterAndExtensions( fileFiltersString, exts, wildcards );
      return fileFiltersString;
    }

    case QgsProviderMetadata::FilterType::FilterVector:
    case QgsProviderMetadata::FilterType::FilterMesh:
    case QgsProviderMetadata::FilterType::FilterMeshDataset:
    case QgsProviderMetadata::FilterType::FilterPointCloud:
      return QString();
  }
  return QString();
}

QString QgsGdalProvider::validateCreationOptions( const QStringList &createOptions, const QString &format )
{
  QString message;

  // first validate basic syntax with GDALValidateCreationOptions
  message = QgsGdalUtils::validateCreationOptionsFormat( createOptions, format );
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
  const auto constCreateOptions = createOptions;
  for ( const QString &option : constCreateOptions )
  {
    QStringList opt = option.split( '=' );
    optionsMap[ opt[0].toUpper()] = opt[1];
    QgsDebugMsgLevel( "option: " + option, 2 );
  }

  // gtiff files - validate PREDICTOR option
  // see gdal: frmts/gtiff/geotiff.cpp and libtiff: tif_predict.c)
  if ( format.compare( QLatin1String( "gtiff" ), Qt::CaseInsensitive ) == 0 && optionsMap.contains( QStringLiteral( "PREDICTOR" ) ) )
  {
    QString value = optionsMap.value( QStringLiteral( "PREDICTOR" ) );
    GDALDataType nDataType = ( !mGdalDataType.isEmpty() ) ? ( GDALDataType ) mGdalDataType.at( 0 ) : GDT_Unknown;
    int nBitsPerSample = nDataType != GDT_Unknown ? GDALGetDataTypeSize( nDataType ) : 0;
    QgsDebugMsgLevel( QStringLiteral( "PREDICTOR: %1 nbits: %2 type: %3" ).arg( value ).arg( nBitsPerSample ).arg( ( GDALDataType ) mGdalDataType.at( 0 ) ), 2 );
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

QgsPoint QgsGdalProvider::transformCoordinates( const QgsPoint &point, QgsRasterDataProvider::TransformType type )
{
  if ( !mGdalTransformerArg )
    return QgsPoint();

  int success;
  double x = point.x(), y = point.y(), z = point.is3D() ? point.z() : 0;
  GDALUseTransformer( mGdalTransformerArg, type == TransformLayerToImage, 1, &x, &y, &z, &success );
  if ( !success )
    return QgsPoint();

  return QgsPoint( x, y, z );
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
    QgsDebugMsgLevel( QStringLiteral( "Waiting for ref counter for %1 to drop to 1" ).arg( dataSourceUri() ), 2 );
    QThread::msleep( 100 );
  }

  closeDataset();

  mUpdate = enabled;

  // reopen the dataset
  mGdalBaseDataset = gdalOpen( dataSourceUri( true ), mUpdate ? GDAL_OF_UPDATE : GDAL_OF_READONLY );
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
QList<QPair<QString, QString> > QgsGdalProviderMetadata::pyramidResamplingMethods()
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
    methods.append( QPair<QString, QString>( QStringLiteral( "BILINEAR" ), QObject::tr( "Bilinear" ) ) );
    methods.append( QPair<QString, QString>( QStringLiteral( "MODE" ), QObject::tr( "Mode" ) ) );
    methods.append( QPair<QString, QString>( QStringLiteral( "NONE" ), QObject::tr( "None" ) ) );
  }

  return methods;
}

QgsProviderMetadata::ProviderMetadataCapabilities QgsGdalProviderMetadata::capabilities() const
{
  return QuerySublayers;
}

QgsProviderMetadata::ProviderCapabilities QgsGdalProviderMetadata::providerCapabilities() const
{
  return FileBasedUris;
}

QList<QgsProviderSublayerDetails> QgsGdalProviderMetadata::querySublayers( const QString &uri, Qgis::SublayerQueryFlags flags, QgsFeedback *feedback ) const
{
  gdal::dataset_unique_ptr dataset;

  QgsGdalProviderBase::registerGdalDrivers();

  QString gdalUri = QgsGdalProvider::expandAuthConfig( uri );

  QVariantMap uriParts = decodeUri( gdalUri );

  // Try to open using VSIFileHandler
  QString vsiPrefix = QgsZipItem::vsiPrefix( gdalUri );
  if ( !vsiPrefix.isEmpty() )
  {
    if ( !gdalUri.startsWith( vsiPrefix ) )
    {
      gdalUri = vsiPrefix + gdalUri;
      uriParts = decodeUri( gdalUri );
    }
  }

  const QString path = uriParts.value( QStringLiteral( "path" ) ).toString();
  const QFileInfo pathInfo( path );
  if ( ( flags & Qgis::SublayerQueryFlag::FastScan ) && ( pathInfo.isFile() || pathInfo.isDir() ) )
  {
    // fast scan, so we don't actually try to open the dataset and instead just check the extension alone
    static QString sFilterString;
    static QStringList sExtensions;
    static QStringList sWildcards;

    // get supported extensions
    static std::once_flag initialized;
    std::call_once( initialized, [ = ]
    {
      buildSupportedRasterFileFilterAndExtensions( sFilterString, sExtensions, sWildcards );
      QgsDebugMsgLevel( QStringLiteral( "extensions: " ) + sExtensions.join( ' ' ), 2 );
      QgsDebugMsgLevel( QStringLiteral( "wildcards: " ) + sWildcards.join( ' ' ), 2 );
    } );

    const QString suffix = uriParts.value( QStringLiteral( "vsiSuffix" ) ).toString().isEmpty()
                           ? pathInfo.suffix().toLower()
                           : QFileInfo( uriParts.value( QStringLiteral( "vsiSuffix" ) ).toString() ).suffix().toLower();

    if ( !sExtensions.contains( suffix ) )
    {
      bool matches = false;
      for ( const QString &wildcard : std::as_const( sWildcards ) )
      {
        const thread_local QRegularExpression rx( QRegularExpression::anchoredPattern(
              QRegularExpression::wildcardToRegularExpression( wildcard )
            ), QRegularExpression::CaseInsensitiveOption );
        const QRegularExpressionMatch match = rx.match( pathInfo.fileName() );
        if ( match.hasMatch() )
        {
          matches = true;
          break;
        }
      }
      if ( !matches )
        return {};
    }

    // if this is a VRT file make sure it is raster VRT
    if ( suffix == QLatin1String( "vrt" ) && !QgsGdalUtils::vrtMatchesLayerType( path, QgsMapLayerType::RasterLayer ) )
    {
      return {};
    }

    // metadata.xml file next to tdenv?.adf files is a subcomponent of an ESRI tin layer alone, shouldn't be exposed
    if ( pathInfo.fileName().compare( QLatin1String( "metadata.xml" ), Qt::CaseInsensitive ) == 0 )
    {
      const QDir dir  = pathInfo.dir();
      if ( dir.exists( QStringLiteral( "tdenv9.adf" ) )
           || dir.exists( QStringLiteral( "tdenv.adf" ) )
           || dir.exists( QStringLiteral( "TDENV9.ADF" ) )
           || dir.exists( QStringLiteral( "TDENV.ADF" ) ) )
        return {};
    }

    QgsProviderSublayerDetails details;
    details.setType( QgsMapLayerType::RasterLayer );
    details.setProviderKey( QStringLiteral( "gdal" ) );
    details.setUri( uri );
    details.setName( uriParts.value( QStringLiteral( "vsiSuffix" ) ).toString().isEmpty()
                     ? QgsProviderUtils::suggestLayerNameFromFilePath( path )
                     : QFileInfo( uriParts.value( QStringLiteral( "vsiSuffix" ) ).toString() ).fileName() );
    if ( QgsGdalUtils::multiLayerFileExtensions().contains( suffix ) )
    {
      // uri may contain sublayers, but query flags prevent us from examining them
      details.setSkippedContainerScan( true );
    }
    return {details};
  }

  if ( !uriParts.value( QStringLiteral( "vsiPrefix" ) ).toString().isEmpty()
       && uriParts.value( QStringLiteral( "vsiSuffix" ) ).toString().isEmpty() )
  {
    // get list of files inside archive file
    QgsDebugMsgLevel( QStringLiteral( "Open file %1 with gdal vsi" ).arg( vsiPrefix + uriParts.value( QStringLiteral( "path" ) ).toString() ), 3 );
    char **papszSiblingFiles = VSIReadDirRecursive( QString( vsiPrefix + uriParts.value( QStringLiteral( "path" ) ).toString() ).toLocal8Bit().constData() );
    if ( papszSiblingFiles )
    {
      QList<QgsProviderSublayerDetails> res;

      QStringList files;
      for ( int i = 0; papszSiblingFiles[i]; i++ )
      {
        files << papszSiblingFiles[i];
      }

      for ( const QString &file : std::as_const( files ) )
      {
        if ( feedback && feedback->isCanceled() )
          break;

        // skip directories (files ending with /)
        if ( file.right( 1 ) != QLatin1String( "/" ) )
        {
          uriParts.insert( QStringLiteral( "vsiSuffix" ), QStringLiteral( "/%1" ).arg( file ) );
          res << querySublayers( encodeUri( uriParts ), flags, feedback );
        }
      }
      CSLDestroy( papszSiblingFiles );
      return res;
    }
  }

  CPLPushErrorHandler( CPLQuietErrorHandler );
  CPLErrorReset();
  dataset.reset( QgsGdalProviderBase::gdalOpen( gdalUri, GDAL_OF_READONLY ) );
  CPLPopErrorHandler();

  if ( !dataset )
  {
    return {};
  }

  const QList< QgsProviderSublayerDetails > res = QgsGdalProvider::sublayerDetails( dataset.get(), uri );
  if ( res.empty() )
  {
    // may not have multiple sublayers, but may still be a single-raster layer dataset
    if ( GDALGetRasterCount( dataset.get() ) != 0 )
    {
      QgsProviderSublayerDetails details;
      details.setProviderKey( PROVIDER_KEY );
      details.setType( QgsMapLayerType::RasterLayer );
      details.setUri( uri );
      details.setLayerNumber( 1 );
      GDALDriverH hDriver = GDALGetDatasetDriver( dataset.get() );
      details.setDriverName( GDALGetDriverShortName( hDriver ) );

      QString name;
      const QVariantMap parts = decodeUri( uri );
      if ( !parts.value( QStringLiteral( "vsiSuffix" ) ).toString().isEmpty() )
      {
        name = parts.value( QStringLiteral( "vsiSuffix" ) ).toString();
        if ( name.startsWith( '/' ) )
          name = name.mid( 1 );
      }
      else if ( parts.contains( QStringLiteral( "path" ) ) )
      {
        name = QgsProviderUtils::suggestLayerNameFromFilePath( parts.value( QStringLiteral( "path" ) ).toString() );
      }
      details.setName( name.isEmpty() ? uri : name );
      return {details};
    }
    else
    {
      return {};
    }
  }
  else
  {
    return res;
  }
}

QStringList QgsGdalProviderMetadata::sidecarFilesForUri( const QString &uri ) const
{
  const QVariantMap uriParts = decodeUri( uri );
  const QString path = uriParts.value( QStringLiteral( "path" ) ).toString();

  if ( path.isEmpty() )
    return {};

  const QFileInfo fileInfo( path );
  const QString suffix = fileInfo.suffix();

  static QMap< QString, QStringList > sExtensions
  {
    {
      QStringLiteral( "jpg" ), {
        QStringLiteral( "jpw" ),
        QStringLiteral( "jgw" ),
        QStringLiteral( "jpgw" ),
        QStringLiteral( "jpegw" ),
      }
    },
    {
      QStringLiteral( "img" ), {
        QStringLiteral( "ige" ),
      }
    },
    {
      QStringLiteral( "sid" ), {
        QStringLiteral( "j2w" ),
      }
    },
    {
      QStringLiteral( "tif" ), {
        QStringLiteral( "tifw" ),
        QStringLiteral( "tfw" ),
      }
    },
    {
      QStringLiteral( "bil" ), {
        QStringLiteral( "bilw" ),
        QStringLiteral( "blw" ),
      }
    },
    {
      QStringLiteral( "raster" ), {
        QStringLiteral( "rasterw" ),
      }
    },
    {
      QStringLiteral( "bt" ), {
        QStringLiteral( "btw" ),
      }
    },
    {
      QStringLiteral( "rst" ), {
        QStringLiteral( "rdc" ),
        QStringLiteral( "smp" ),
        QStringLiteral( "ref" ),
        QStringLiteral( "vct" ),
        QStringLiteral( "vdc" ),
        QStringLiteral( "avl" ),
      }
    },
    {
      QStringLiteral( "sdat" ), {
        QStringLiteral( "sgrd" ),
        QStringLiteral( "mgrd" ),
        QStringLiteral( "prj" ),
      }
    }
  };


  QStringList res;
  // format specific sidecars
  for ( auto it = sExtensions.constBegin(); it != sExtensions.constEnd(); ++it )
  {
    if ( suffix.compare( it.key(), Qt::CaseInsensitive ) == 0 )
    {
      for ( const QString &ext : it.value() )
      {
        res.append( fileInfo.dir().filePath( fileInfo.completeBaseName() + '.' + ext ) );
      }
    }
  }

  // sidecars which could be present for any file
  for ( const QString &ext :
        {
          QStringLiteral( "aux.xml" ),
          QStringLiteral( "vat.dbf" ),
          QStringLiteral( "ovr" ),
          QStringLiteral( "wld" ),
        } )
  {
    res.append( fileInfo.dir().filePath( fileInfo.completeBaseName() + '.' + ext ) );
    res.append( path + '.' + ext );
  }
  return res;
}

QgsGdalProviderMetadata::QgsGdalProviderMetadata():
  QgsProviderMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION )
{
}

///@endcond

