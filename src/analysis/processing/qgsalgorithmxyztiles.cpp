/***************************************************************************
                         qgsalgorithmxyztiles.cpp
                         ---------------------
    begin                : August 2023
    copyright            : (C) 2023 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmxyztiles.h"

#include <atomic>

#include "qgsexpressioncontextutils.h"
#include "qgsimageoperation.h"
#include "qgslayertree.h"
#include "qgslayertreelayer.h"
#include "qgsmaplayerutils.h"
#include "qgsmaprenderercustompainterjob.h"

#include <QBuffer>
#include <QDir>
#include <QQueue>
#include <QSemaphore>
#include <QSet>
#include <QString>
#include <QThread>
#include <QThreadPool>
#include <QWaitCondition>

using namespace Qt::StringLiterals;

///@cond PRIVATE


namespace
{
  int tile2tms( const int y, const int zoom )
  {
    double n = std::pow( 2, zoom );
    return ( int ) std::floor( n - y - 1 );
  }

  int lon2tileX( const double lon, const int z )
  {
    return ( int ) ( std::floor( ( lon + 180.0 ) / 360.0 * ( 1 << z ) ) );
  }

  int lat2tileY( const double lat, const int z )
  {
    double latRad = lat * M_PI / 180.0;
    return ( int ) ( std::floor( ( 1.0 - std::asinh( std::tan( latRad ) ) / M_PI ) / 2.0 * ( 1 << z ) ) );
  }

  double tileX2lon( const int x, const int z )
  {
    return x / ( double ) ( 1 << z ) * 360.0 - 180;
  }

  double tileY2lat( const int y, const int z )
  {
    double n = M_PI - 2.0 * M_PI * y / ( double ) ( 1 << z );
    return 180.0 / M_PI * std::atan( 0.5 * ( std::exp( n ) - std::exp( -n ) ) );
  }
} //namespace


void MetaTile::addTile( const int row, const int col, Tile tileToAdd )
{
  tiles.insert( QPair<int, int>( row, col ), tileToAdd );
  if ( row >= rows )
  {
    rows = row + 1;
  }
  if ( col >= cols )
  {
    cols = col + 1;
  }
}

QgsRectangle MetaTile::extent() const
{
  const Tile first = tiles.first();
  const Tile last = tiles.last();
  return QgsRectangle( tileX2lon( first.x, first.z ), tileY2lat( last.y + 1, last.z ), tileX2lon( last.x + 1, last.z ), tileY2lat( first.y, first.z ) );
}


namespace
{
  QList<MetaTile> getMetatiles( const QgsRectangle extent, const int zoom, long long &tileCount, const int tileSize )
  {
    int minX = lon2tileX( extent.xMinimum(), zoom );
    int minY = lat2tileY( extent.yMaximum(), zoom );
    int maxX = lon2tileX( extent.xMaximum(), zoom );
    int maxY = lat2tileY( extent.yMinimum(), zoom );
    tileCount = static_cast<long long>( maxX - minX + 1 ) * static_cast<long long>( maxY - minY + 1 );

    QHash<uint64_t, MetaTile> tiles;
    int i = 0;
    for ( int x = minX; x <= maxX; x++ )
    {
      int j = 0;
      for ( int y = minY; y <= maxY; y++ )
      {
        const uint64_t key = ( static_cast<uint64_t>( i / tileSize ) << 32 ) | static_cast<uint32_t>( j / tileSize );
        tiles[key].addTile( i % tileSize, j % tileSize, Tile( x, y, zoom ) );
        j++;
      }
      i++;
    }
    return tiles.values();
  }
} //namespace

/**
 * Queue for rendered tiles to write to a database.
 *
 * (This acts effectively the "consumer" from the producer/consumer pattern.)
 *
 * Rendered tiles can be consumed from the queue in batches via popBatch(), meaning
 * that we can minimize the number of database writes we do by writing thousands
 * in a single pass.
 */
class PendingTilesToWriteQueue
{
  public:
    /**
     * Pushes a list of rendered tiles from a metatile to the queue.
     */
    void push( const QList<QgsMbTiles::TileData> &tiles )
    {
      if ( tiles.isEmpty() )
        return;

      QMutexLocker locker( &mMutex );
      for ( const QgsMbTiles::TileData &tile : tiles )
      {
        mQueue.enqueue( tile );
      }
      mNotEmpty.wakeOne();
    }

    /**
     * Pops a batch of rendered tiles of the specified maximum batch size.
     *
     * If rendering finishes before the batch size is reached (i.e. a call
     * to setFinished() is made), or if the specified maximum timeout elapses,
     * then a batch smaller then \a maxBatchSize will be returned.
     */
    bool popBatch( QList<QgsMbTiles::TileData> &batch, int maxBatchSize, unsigned long timeoutMs = 500 )
    {
      QMutexLocker locker( &mMutex );

      // wait until there is at least one item or all rendering is finished
      while ( mQueue.isEmpty() && !mFinished )
      {
        mNotEmpty.wait( &mMutex );
      }

      if ( mQueue.isEmpty() )
        return false;

      // collect up to to maxBatchSize, unless we finish rendering or timeout before that happens
      while ( mQueue.size() < maxBatchSize && !mFinished )
      {
        if ( !mNotEmpty.wait( &mMutex, timeoutMs ) )
        {
          // timeout exceeded
          break;
        }
      }

      while ( !mQueue.isEmpty() && batch.size() < maxBatchSize )
      {
        batch.append( mQueue.dequeue() );
      }
      return true;
    }

    /**
     * Sets the queue as finished.
     */
    void setFinished()
    {
      QMutexLocker locker( &mMutex );
      mFinished = true;
      mNotEmpty.wakeAll();
    }

  private:
    QQueue<QgsMbTiles::TileData> mQueue;
    mutable QMutex mMutex;
    QWaitCondition mNotEmpty;
    bool mFinished = false;
};

//
// QgsXyzTilesBaseAlgorithm
//

QString QgsXyzTilesBaseAlgorithm::group() const
{
  return QObject::tr( "Raster tools" );
}

QString QgsXyzTilesBaseAlgorithm::groupId() const
{
  return u"rastertools"_s;
}

Qgis::ProcessingAlgorithmFlags QgsXyzTilesBaseAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::RequiresProject;
}

void QgsXyzTilesBaseAlgorithm::createCommonParameters()
{
  auto extentParam = std::make_unique<QgsProcessingParameterExtent>( u"EXTENT"_s, QObject::tr( "Extent" ) );
  extentParam->setHelp( QObject::tr( "Spatial extent of the area for tile generation." ) );
  addParameter( extentParam.release() );

  auto minZoomParam = std::make_unique<QgsProcessingParameterNumber>( u"ZOOM_MIN"_s, QObject::tr( "Minimum zoom" ), Qgis::ProcessingNumberParameterType::Integer, 12, false, 0, 25 );
  minZoomParam->setHelp(
    QObject::tr(
      "Minimum zoom level for generated tiles (0–25). Lower zoom levels cover broader geographic areas "
      "at lower spatial resolution. Must be less than or equal to the maximum zoom level."
    )
  );
  addParameter( minZoomParam.release() );

  auto maxZoomParam = std::make_unique<QgsProcessingParameterNumber>( u"ZOOM_MAX"_s, QObject::tr( "Maximum zoom" ), Qgis::ProcessingNumberParameterType::Integer, 12, false, 0, 25 );
  maxZoomParam->setHelp(
    QObject::tr(
      "Maximum zoom level for generated tiles (0–25). Higher zoom levels capture finer map details "
      "and higher resolution, but exponentially increase total tile count, storage requirements, and rendering time. "
      "Must be greater than or equal to the minimum zoom level."
    )
  );
  addParameter( maxZoomParam.release() );

  auto dpiParam = std::make_unique<QgsProcessingParameterNumber>( u"DPI"_s, QObject::tr( "DPI" ), Qgis::ProcessingNumberParameterType::Integer, 96, false, 48, 600 );
  dpiParam->setHelp( QObject::tr( "Output resolution in DPI for rendered map content." ) );
  dpiParam->setFlags( dpiParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( dpiParam.release() );

  auto bgColorParam = std::make_unique<QgsProcessingParameterColor>( u"BACKGROUND_COLOR"_s, QObject::tr( "Background color" ), QColor( Qt::transparent ), true, true );
  bgColorParam->setHelp( QObject::tr( "Background color used when rendering map tiles." ) );
  addParameter( bgColorParam.release() );

  auto antialiasParam = std::make_unique<QgsProcessingParameterBoolean>( u"ANTIALIAS"_s, QObject::tr( "Enable antialiasing" ), true );
  antialiasParam->setHelp( QObject::tr( "Controls whether antialiasing is applied during tile rendering." ) );
  antialiasParam->setFlags( antialiasParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( antialiasParam.release() );

  auto tileFormatParam = std::make_unique<QgsProcessingParameterEnum>( u"TILE_FORMAT"_s, QObject::tr( "Tile format" ), QStringList { u"PNG"_s, u"JPG"_s, u"WEBP"_s }, false, 0 );
  tileFormatParam->setHelp( QObject::tr( "Output image format for the rendered tiles." ) );
  addParameter( tileFormatParam.release() );

  auto qualityParam = std::make_unique<QgsProcessingParameterNumber>( u"QUALITY"_s, QObject::tr( "Quality (JPG only)" ), Qgis::ProcessingNumberParameterType::Integer, 75, false, 1, 100 );
  qualityParam->setHelp( QObject::tr( "Image quality percentage used when tile format is set to JPG (1–100)." ) );
  qualityParam->setFlags( qualityParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( qualityParam.release() );

  auto metaTileSizeParam = std::make_unique<QgsProcessingParameterNumber>( u"METATILESIZE"_s, QObject::tr( "Metatile size" ), Qgis::ProcessingNumberParameterType::Integer, 4, false, 1, 20 );
  metaTileSizeParam->setHelp( QObject::tr( "Size of metatiles (in tile units) used during rendering." ) );
  metaTileSizeParam->setFlags( metaTileSizeParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( metaTileSizeParam.release() );

  auto skipEmptyTilesParam = std::make_unique<QgsProcessingParameterBoolean>( u"SKIP_EMPTY_TILES"_s, QObject::tr( "Skip empty tiles" ), false );
  skipEmptyTilesParam->setHelp( QObject::tr( "If set, completely empty tiles will be skipped." ) );
  skipEmptyTilesParam->setFlags( skipEmptyTilesParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( skipEmptyTilesParam.release() );
}

bool QgsXyzTilesBaseAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsProject *project = context.project();

  mExpressionContext = context.expressionContext();

  const QList<QgsLayerTreeLayer *> projectLayers = project->layerTreeRoot()->findLayers();
  QSet<QString> visibleLayers;
  for ( const QgsLayerTreeLayer *layer : projectLayers )
  {
    if ( layer->isVisible() )
    {
      visibleLayers << layer->layer()->id();
    }
  }

  const QList<QgsMapLayer *> renderLayers = project->layerTreeRoot()->layerOrder();
  for ( QgsMapLayer *layer : renderLayers )
  {
    if ( visibleLayers.contains( layer->id() ) )
    {
      QgsMapLayer *clonedLayer = layer->clone();
      clonedLayer->moveToThread( nullptr );
      mLayers << clonedLayer;
    }
  }

  QgsRectangle extent = parameterAsExtent( parameters, u"EXTENT"_s, context );
  QgsCoordinateReferenceSystem extentCrs = parameterAsExtentCrs( parameters, u"EXTENT"_s, context );
  QgsCoordinateTransform ct( extentCrs, project->crs(), context.transformContext() );
  ct.setBallparkTransformsAreAppropriate( true );
  try
  {
    mExtent = ct.transformBoundingBox( extent );
  }
  catch ( QgsCsException & )
  {
    throw QgsProcessingException( QObject::tr( "Could not transform the extent into the project CRS" ) );
  }

  mMinZoom = parameterAsInt( parameters, u"ZOOM_MIN"_s, context );
  mMaxZoom = parameterAsInt( parameters, u"ZOOM_MAX"_s, context );
  if ( mMaxZoom < mMinZoom )
  {
    throw QgsProcessingException( QObject::tr( "Maximum zoom (%1) must be ≥ minimum zoom (%2)" ).arg( mMaxZoom ).arg( mMinZoom ) );
  }
  mDpi = parameterAsInt( parameters, u"DPI"_s, context );
  mBackgroundColor = parameterAsColor( parameters, u"BACKGROUND_COLOR"_s, context );
  mAntialias = parameterAsBool( parameters, u"ANTIALIAS"_s, context );
  mSkipEmptyTiles = parameterAsBool( parameters, u"SKIP_EMPTY_TILES"_s, context );
  switch ( parameterAsEnum( parameters, u"TILE_FORMAT"_s, context ) )
  {
    case 0:
      mTileFormat = u"PNG"_s;
      break;
    case 1:
      mTileFormat = u"JPG"_s;
      break;
    case 2:
      mTileFormat = u"WEBP"_s;
      break;
    default:
      mTileFormat = u"PNG"_s;
      break;
  }

  mJpgQuality = mTileFormat != "PNG"_L1 ? parameterAsInt( parameters, u"QUALITY"_s, context ) : -1;
  mMetaTileSize = parameterAsInt( parameters, u"METATILESIZE"_s, context );
  mThreadsNumber = context.maximumThreads();
  mTransformContext = context.transformContext();
  mEllipsoid = context.ellipsoid();

  QgsCoordinateTransform src2Wgs = QgsCoordinateTransform( project->crs(), QgsCoordinateReferenceSystem( "EPSG:4326" ), context.transformContext() );
  src2Wgs.setBallparkTransformsAreAppropriate( true );
  try
  {
    mWgs84Extent = src2Wgs.transformBoundingBox( mExtent );
  }
  catch ( QgsCsException & )
  {
    throw QgsProcessingException( QObject::tr( "Could not transform the extent into WGS84" ) );
  }

  if ( parameters.contains( u"TILE_WIDTH"_s ) )
  {
    mTileWidth = parameterAsInt( parameters, u"TILE_WIDTH"_s, context );
  }

  if ( parameters.contains( u"TILE_HEIGHT"_s ) )
  {
    mTileHeight = parameterAsInt( parameters, u"TILE_HEIGHT"_s, context );
  }

  if ( ( mTileFormat != "PNG"_L1 && mTileFormat != "WEBP"_L1 ) && mBackgroundColor.alpha() != 255 )
  {
    feedback->pushWarning(
      QObject::tr( "A semi-transparent background color was set, but the JPG format only supports fully opaque colors. The background color setting will be ignored. Please use a fully opaque background color instead." )
    );
  }

  mScaleMethod = project->scaleMethod();

  return true;
}

void QgsXyzTilesBaseAlgorithm::checkLayersUsagePolicy( QgsProcessingFeedback *feedback )
{
  if ( mTotalMetaTiles > MAXIMUM_OPENSTREETMAP_TILES_FETCH )
  {
    for ( QgsMapLayer *layer : std::as_const( mLayers ) )
    {
      if ( QgsMapLayerUtils::isOpenStreetMapLayer( layer ) )
      {
        // Prevent bulk downloading of tiles from openstreetmap.org as per OSMF tile usage policy
        feedback->pushFormattedMessage(
          QObject::tr( "Layer %1 will be skipped as the algorithm leads to bulk downloading behavior which is prohibited by the %2OpenStreetMap Foundation tile usage policy%3" )
            .arg( layer->name(), u"<a href=\"https://operations.osmfoundation.org/policies/tiles/\">"_s, u"</a>"_s ),
          QObject::tr( "Layer %1 will be skipped as the algorithm leads to bulk downloading behavior which is prohibited by the %2OpenStreetMap Foundation tile usage policy%3" )
            .arg( layer->name(), QString(), QString() )
        );
        mLayers.removeAll( layer );
        delete layer;
      }
    }
  }
}

std::optional< QgsMapSettings > QgsXyzTilesBaseAlgorithm::mapSettingsForTile( const MetaTile &metaTile ) const
{
  QgsCoordinateReferenceSystem mercatorCrs = QgsCoordinateReferenceSystem( "EPSG:3857" );
  QgsCoordinateTransform wgsToMercator = QgsCoordinateTransform( QgsCoordinateReferenceSystem( "EPSG:4326" ), mercatorCrs, mTransformContext );
  wgsToMercator.setBallparkTransformsAreAppropriate( true );

  QgsMapSettings settings;
  try
  {
    settings.setExtent( wgsToMercator.transformBoundingBox( metaTile.extent() ) );
  }
  catch ( QgsCsException & )
  {
    return {};
  }
  settings.setRendererUsage( Qgis::RendererUsage::Export );
  settings.setOutputImageFormat( QImage::Format_ARGB32_Premultiplied );
  settings.setTransformContext( mTransformContext );
  settings.setEllipsoid( mEllipsoid );
  settings.setDestinationCrs( mercatorCrs );
  settings.setLayers( mLayers );
  settings.setOutputDpi( mDpi );
  settings.setFlag( Qgis::MapSettingsFlag::Antialiasing, mAntialias );
  settings.setFlag( Qgis::MapSettingsFlag::RenderMapTile, true );
  settings.setFlag( Qgis::MapSettingsFlag::UseRenderingOptimization, true );
  settings.setFlag( Qgis::MapSettingsFlag::HighQualityImageTransforms, true );
  settings.setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy::Default );
  settings.setScaleMethod( mScaleMethod );
  if ( mTileFormat == "PNG"_L1 || mTileFormat == "WEBP"_L1 || mBackgroundColor.alpha() == 255 )
  {
    settings.setBackgroundColor( mBackgroundColor );
  }
  QSize size( mTileWidth * metaTile.rows, mTileHeight * metaTile.cols );
  settings.setOutputSize( size );

  QgsLabelingEngineSettings labelingSettings = settings.labelingEngineSettings();
  labelingSettings.setFlag( Qgis::LabelingFlag::UsePartialCandidates, false );
  settings.setLabelingEngineSettings( labelingSettings );

  QgsExpressionContext exprContext = mExpressionContext;
  exprContext.appendScope( QgsExpressionContextUtils::mapSettingsScope( settings ) );
  settings.setExpressionContext( exprContext );

  return settings;
}

void QgsXyzTilesBaseAlgorithm::startJobs( QgsProcessingFeedback *feedback )
{
  while ( mRendererJobs.size() < mThreadsNumber && !mMetaTiles.empty() )
  {
    if ( feedback->isCanceled() )
      break;

    MetaTile metaTile = mMetaTiles.takeFirst();
    const std::optional<QgsMapSettings> settings = mapSettingsForTile( metaTile );
    if ( !settings.has_value() )
    {
      mProcessedMetaTiles++;
      feedback->setProgress( 100.0 * mProcessedMetaTiles / mTotalMetaTiles );
      continue;
    }

    QgsMapRendererSequentialJob *job = new QgsMapRendererSequentialJob( *settings );
    mRendererJobs.insert( job, metaTile );

    QObject::connect( job, &QgsMapRendererJob::finished, mJobOwner, [this, feedback, job]() {
      const MetaTile tile = mRendererJobs.take( job );
      const QImage renderedImage = job->renderedImage();
      job->deleteLater();

      mProcessedMetaTiles++;
      feedback->setProgress( 100.0 * mProcessedMetaTiles / mTotalMetaTiles );

      processMetaTile( tile, renderedImage, feedback );

      if ( !feedback->isCanceled() )
      {
        startJobs( feedback );
      }
      checkPipelineFinished( feedback );
    } );

    job->start();
  }

  checkPipelineFinished( feedback );
}

void QgsXyzTilesBaseAlgorithm::checkPipelineFinished( QgsProcessingFeedback *feedback )
{
  if ( feedback->isCanceled() || ( mMetaTiles.isEmpty() && mRendererJobs.isEmpty() && mActivePostProcessingTasks == 0 ) )
  {
    if ( mEventLoop )
    {
      mEventLoop->exit();
    }
  }
}

//
// QgsXyzTilesDirectoryAlgorithm
//

QString QgsXyzTilesDirectoryAlgorithm::name() const
{
  return u"tilesxyzdirectory"_s;
}

QString QgsXyzTilesDirectoryAlgorithm::displayName() const
{
  return QObject::tr( "Generate XYZ tiles (Directory)" );
}

QStringList QgsXyzTilesDirectoryAlgorithm::tags() const
{
  return QObject::tr( "tiles,xyz,tms,directory" ).split( ',' );
}

QString QgsXyzTilesDirectoryAlgorithm::shortHelpString() const
{
  return QObject::tr(
    "This algorithm generates XYZ raster tiles from the current project and saves them as individual image files in a structured directory hierarchy ({z}/{x}/{y}.png or .jpg).\n\n"
    "All visible map layers from the project will be rendered into tiles across the specified extent and zoom range.\n\n"
    "Optionally, a standalone Leaflet HTML file can be generated for instant web previewing of the tiles."
  );
}

QString QgsXyzTilesDirectoryAlgorithm::shortDescription() const
{
  return QObject::tr( "Generates XYZ tiles from the project as a structured directory hierarchy ({z}/{x}/{y}.png/jpg)." );
}

QgsXyzTilesDirectoryAlgorithm *QgsXyzTilesDirectoryAlgorithm::createInstance() const
{
  return new QgsXyzTilesDirectoryAlgorithm();
}

void QgsXyzTilesDirectoryAlgorithm::initAlgorithm( const QVariantMap & )
{
  createCommonParameters();
  auto tileWidthParam = std::make_unique<QgsProcessingParameterNumber>( u"TILE_WIDTH"_s, QObject::tr( "Tile width" ), Qgis::ProcessingNumberParameterType::Integer, 256, false, 1, 4096 );
  tileWidthParam->setHelp( QObject::tr( "Width of each tile image in pixels." ) );
  addParameter( tileWidthParam.release() );

  auto tileHeightParam = std::make_unique<QgsProcessingParameterNumber>( u"TILE_HEIGHT"_s, QObject::tr( "Tile height" ), Qgis::ProcessingNumberParameterType::Integer, 256, false, 1, 4096 );
  tileHeightParam->setHelp( QObject::tr( "Height of each tile image in pixels." ) );
  addParameter( tileHeightParam.release() );

  auto tmsParam = std::make_unique<QgsProcessingParameterBoolean>( u"TMS_CONVENTION"_s, QObject::tr( "Use inverted tile Y axis (TMS convention)" ), false );
  tmsParam->setHelp( QObject::tr( "Inverts the Y tile coordinate naming convention to follow TMS format." ) );
  addParameter( tmsParam.release() );

  auto titleParam = std::make_unique<QgsProcessingParameterString>( u"HTML_TITLE"_s, QObject::tr( "Leaflet HTML output title" ), QVariant(), false, true );
  titleParam->setFlags( titleParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  titleParam->setHelp( QObject::tr( "Title displayed in the generated Leaflet HTML web viewer." ) );
  addParameter( titleParam.release() );

  auto attributionParam = std::make_unique<QgsProcessingParameterString>( u"HTML_ATTRIBUTION"_s, QObject::tr( "Leaflet HTML output attribution" ), QVariant(), false, true );
  attributionParam->setFlags( attributionParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  attributionParam->setHelp( QObject::tr( "Attribution text displayed in the generated Leaflet HTML web viewer." ) );
  addParameter( attributionParam.release() );

  auto osmParam = std::make_unique<QgsProcessingParameterBoolean>( u"HTML_OSM"_s, QObject::tr( "Include OpenStreetMap basemap in Leaflet HTML output" ), false );
  osmParam->setFlags( osmParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  osmParam->setHelp( QObject::tr( "Includes an OpenStreetMap background layer in the generated Leaflet HTML viewer." ) );
  addParameter( osmParam.release() );

  auto outputDirParam = std::make_unique<QgsProcessingParameterFolderDestination>( u"OUTPUT_DIRECTORY"_s, QObject::tr( "Output directory" ) );
  outputDirParam->setHelp( QObject::tr( "Destination folder where the generated directory structure and tile files will be stored." ) );
  addParameter( outputDirParam.release() );

  auto outputHtmlParam = std::make_unique<QgsProcessingParameterFileDestination>( u"OUTPUT_HTML"_s, QObject::tr( "Output HTML (Leaflet)" ), QObject::tr( "HTML files (*.html)" ), QVariant(), true );
  outputHtmlParam->setHelp( QObject::tr( "Destination file path for the optional Leaflet HTML web map preview." ) );
  addParameter( outputHtmlParam.release() );

  addOutput( new QgsProcessingOutputRasterLayer( u"OUTPUT_LAYER"_s, QObject::tr( "Output tiles as raster layer" ) ) );
}

QVariantMap QgsXyzTilesDirectoryAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QGS_MARK_ALGORITHM_SOURCE

  const bool tms = parameterAsBoolean( parameters, u"TMS_CONVENTION"_s, context );
  const QString title = parameterAsString( parameters, u"HTML_TITLE"_s, context );
  const QString attribution = parameterAsString( parameters, u"HTML_ATTRIBUTION"_s, context );
  const bool useOsm = parameterAsBoolean( parameters, u"HTML_OSM"_s, context );
  QString outputDir = parameterAsString( parameters, u"OUTPUT_DIRECTORY"_s, context );
  const QString outputHtml = parameterAsString( parameters, u"OUTPUT_HTML"_s, context );

  mOutputDir = outputDir;
  mTms = tms;

  long long totalTiles = 0;
  mTotalMetaTiles = 0;
  for ( int z = mMinZoom; z <= mMaxZoom; z++ )
  {
    if ( feedback->isCanceled() )
      break;

    long long tileCount = 0;
    mMetaTiles += getMetatiles( mWgs84Extent, z, tileCount, mMetaTileSize );
    feedback->pushInfo( QObject::tr( "%1 metatiles (%2 tiles) will be created for zoom level %3" ).arg( mMetaTiles.size() - mTotalMetaTiles ).arg( tileCount ).arg( z ) );
    mTotalMetaTiles = mMetaTiles.size();
    totalTiles += tileCount;
  }
  if ( mTotalMetaTiles == 0 )
  {
    throw QgsProcessingException( QObject::tr( "No metatiles will be created -- please check the extent and zoom limits" ) );
  }

  feedback->pushInfo( QObject::tr( "A total of %1 metatiles (%2 tiles) will be created" ).arg( mTotalMetaTiles ).arg( totalTiles ) );

  checkLayersUsagePolicy( feedback );

  for ( QgsMapLayer *layer : std::as_const( mLayers ) )
  {
    layer->moveToThread( QThread::currentThread() );
  }

  doExport( feedback );

  qDeleteAll( mLayers );
  mLayers.clear();

  if ( mSkipEmptyTiles )
  {
    feedback->pushInfo( QObject::tr( "Wrote %1 total tiles, skipped %2 empty tiles" ).arg( mTilesWritten.load() ).arg( mEmptyTiles.load() ) );
  }

  QVariantMap results;
  results.insert( u"OUTPUT_DIRECTORY"_s, outputDir );

  if ( !outputHtml.isEmpty() )
  {
    const QString osm = QStringLiteral(
                          "var osm_layer = L.tileLayer('https://tile.openstreetmap.org/{z}/{x}/{y}.png',"
                          "{minZoom: %1, maxZoom: %2, attribution: '&copy; <a href=\"https://www.openstreetmap.org/copyright\">OpenStreetMap</a> contributors'}).addTo(map);"
    )
                          .arg( mMinZoom )
                          .arg( mMaxZoom );

    const QString addOsm = useOsm ? osm : QString();
    const QString tmsConvention = tms ? u"true"_s : u"false"_s;
    const QString attr = attribution.isEmpty() ? u"Created by QGIS"_s : attribution;
    const QString tileSource = u"'file:///%1/{z}/{x}/{y}.%2'"_s.arg( outputDir.replace( "\\", "/" ).toHtmlEscaped(), mTileFormat.toLower() );

    const QString html = QStringLiteral(
                           "<!DOCTYPE html><html><head><title>%1</title><meta charset=\"utf-8\"/>"
                           "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
                           "<link rel=\"stylesheet\" href=\"https://unpkg.com/leaflet@1.9.4/dist/leaflet.css\""
                           "integrity=\"sha384-sHL9NAb7lN7rfvG5lfHpm643Xkcjzp4jFvuavGOndn6pjVqS6ny56CAt3nsEVT4H\""
                           "crossorigin=\"\"/>"
                           "<script src=\"https://unpkg.com/leaflet@1.9.4/dist/leaflet.js\""
                           "integrity=\"sha384-cxOPjt7s7Iz04uaHJceBmS+qpjv2JkIHNVcuOrM+YHwZOmJGBXI00mdUXEq65HTH\""
                           "crossorigin=\"\"></script>"
                           "<style type=\"text/css\">body {margin: 0;padding: 0;} html, body, #map{width: 100%;height: 100%;}</style></head>"
                           "<body><div id=\"map\"></div><script>"
                           "var map = L.map('map', {attributionControl: false}).setView([%2, %3], %4);"
                           "L.control.attribution({prefix: false}).addTo(map);"
                           "%5"
                           "var tilesource_layer = L.tileLayer(%6, {minZoom: %7, maxZoom: %8, tms: %9, attribution: '%10'}).addTo(map);"
                           "</script></body></html>"
    )
                           .arg( title.isEmpty() ? u"Leaflet preview"_s : title )
                           .arg( mWgs84Extent.center().y() )
                           .arg( mWgs84Extent.center().x() )
                           .arg( ( mMaxZoom + mMinZoom ) / 2 )
                           .arg( addOsm, tileSource )
                           .arg( mMinZoom )
                           .arg( mMaxZoom )
                           .arg( tmsConvention, attr );

    QFile htmlFile( outputHtml );
    if ( !htmlFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    {
      throw QgsProcessingException( QObject::tr( "Could not open html file %1" ).arg( outputHtml ) );
    }
    QTextStream fout( &htmlFile );
    fout << html;

    results.insert( u"OUTPUT_HTML"_s, outputHtml );
  }

  // try to load the result as a raster layer
  if ( !feedback->isCanceled() )
  {
    const QString layerUri
      = u"type=xyz&url=file:///%1/%7Bz%7D/%7Bx%7D/%7By%7D.%2&zmax=%3&zmin=%4"_s.arg( outputDir.replace( "\\", "/" ).toHtmlEscaped(), mTileFormat.toLower() ).arg( mMaxZoom ).arg( mMinZoom );
    auto layer = std::make_unique<QgsRasterLayer>( layerUri, "OUTPUT_LAYER", u"wms"_s );
    if ( !layer->isValid() )
    {
      feedback->reportError( QObject::tr( "Failed to open XYZ directory as a raster layer" ) );
    }
    const QString layerId = layer->id();
    const QgsProcessingContext::LayerDetails details( layer->name(), context.project(), u"OUTPUT_LAYER"_s, QgsProcessingUtils::LayerHint::Raster );
    details.setOutputLayerName( layer.get() );
    context.addLayerToLoadOnCompletion( layerId, details );
    context.temporaryLayerStore()->addMapLayer( layer.release() );
    results.insert( u"OUTPUT_LAYER"_s, layerId );
  }

  return results;
}

void QgsXyzTilesDirectoryAlgorithm::processMetaTile( const MetaTile &metaTile, const QImage &renderedImage, QgsProcessingFeedback *feedback )
{
  mActivePostProcessingTasks++;

  mPostProcessingPool->start( [this, metaTile, renderedImage, feedback]() {
    const bool testEmptyTilesUsingAlpha0 = mTileFormat != "JPG"_L1 && mBackgroundColor.alpha() == 0;
    long long localWritten = 0;
    long long localEmpty = 0;

    QSet<QString> createdDirs;

    for ( auto it = metaTile.tiles.constBegin(); it != metaTile.tiles.constEnd(); ++it )
    {
      if ( feedback->isCanceled() )
        break;

      const QPair<int, int> tm = it.key();
      const QImage tileImage = renderedImage.copy( mTileWidth * tm.first, mTileHeight * tm.second, mTileWidth, mTileHeight );
      const bool skipTile = mSkipEmptyTiles && ( testEmptyTilesUsingAlpha0 ? QgsImageOperation::isBlankImage( tileImage ) : QgsImageOperation::isSingleColor( tileImage, mBackgroundColor ) );
      if ( skipTile )
      {
        localEmpty++;
        continue;
      }

      const Tile tile = it.value();
      const QString dirPath = u"%1/%2/%3"_s.arg( mOutputDir ).arg( tile.z ).arg( tile.x );
      if ( !createdDirs.contains( dirPath ) )
      {
        QDir().mkpath( dirPath );
        createdDirs.insert( dirPath );
      }

      const int y = mTms ? tile2tms( tile.y, tile.z ) : tile.y;
      const QString filePath = u"%1/%2.%3"_s.arg( dirPath ).arg( y ).arg( mTileFormat.toLower() );
      tileImage.save( filePath, mTileFormat.toStdString().c_str(), mJpgQuality );

      localWritten++;
    }

    mTilesWritten += localWritten;
    mEmptyTiles += localEmpty;

    mActivePostProcessingTasks--;
    checkPipelineFinished( feedback );
  } );
}

void QgsXyzTilesDirectoryAlgorithm::doExport( QgsProcessingFeedback *feedback )
{
  mPostProcessingPool = std::make_unique<QThreadPool>();
  mPostProcessingPool->setMaxThreadCount( std::max( 1, mThreadsNumber ) );

  QEventLoop loop;
  mEventLoop = &loop;
  mJobOwner.reset( new QObject() );

  startJobs( feedback );

  if ( !mMetaTiles.isEmpty() || !mRendererJobs.isEmpty() || mActivePostProcessingTasks.load() > 0 )
  {
    loop.exec();
  }

  for ( auto it = mRendererJobs.constBegin(); it != mRendererJobs.constEnd(); it++ )
  {
    it.key()->cancel();
    it.key()->deleteLater();
  }
  mRendererJobs.clear();

  mPostProcessingPool->waitForDone();
  mPostProcessingPool.reset();
  mEventLoop = nullptr;
}


//
// QgsXyzTilesMbtilesAlgorithm
//

QString QgsXyzTilesMbtilesAlgorithm::name() const
{
  return u"tilesxyzmbtiles"_s;
}

QString QgsXyzTilesMbtilesAlgorithm::displayName() const
{
  return QObject::tr( "Generate XYZ tiles (MBTiles)" );
}

QStringList QgsXyzTilesMbtilesAlgorithm::tags() const
{
  return QObject::tr( "tiles,xyz,tms,mbtiles" ).split( ',' );
}

QString QgsXyzTilesMbtilesAlgorithm::shortHelpString() const
{
  return QObject::tr(
    "This algorithm generates XYZ raster tiles from the current project and packages them into a single, portable MBTiles (SQLite) database file.\n\n"
    "All visible map layers from the project will be rendered into tiles across the specified extent and zoom range."
  );
}

QString QgsXyzTilesMbtilesAlgorithm::shortDescription() const
{
  return QObject::tr( "Generates XYZ tiles from the project as a single, portable MBTiles (SQLite) database." );
}


QgsXyzTilesMbtilesAlgorithm *QgsXyzTilesMbtilesAlgorithm::createInstance() const
{
  return new QgsXyzTilesMbtilesAlgorithm();
}

void QgsXyzTilesMbtilesAlgorithm::initAlgorithm( const QVariantMap & )
{
  createCommonParameters();
  addParameter( new QgsProcessingParameterFileDestination( u"OUTPUT_FILE"_s, QObject::tr( "Output" ), QObject::tr( "MBTiles files (*.mbtiles *.MBTILES)" ) ) );

  addOutput( new QgsProcessingOutputRasterLayer( u"OUTPUT_LAYER"_s, QObject::tr( "Output MBTiles raster layer" ) ) );
}

QVariantMap QgsXyzTilesMbtilesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QGS_MARK_ALGORITHM_SOURCE

  const QString outputFile = parameterAsString( parameters, u"OUTPUT_FILE"_s, context );
  if ( QFile::exists( outputFile ) )
  {
    feedback->pushWarning( QObject::tr( "Removing existing file '%1'" ).arg( QDir::toNativeSeparators( outputFile ) ) );
    if ( !QFile( outputFile ).remove() )
    {
      throw QgsProcessingException( QObject::tr( "Could not remove existing file '%1'" ).arg( QDir::toNativeSeparators( outputFile ) ) );
    }
  }

  mMbtilesWriter = std::make_unique<QgsMbTiles>( outputFile );
  // use deferred index creation, as we'll be writing 1000s of tiles and don't want to update
  // the index after every one
  if ( !mMbtilesWriter->create( true ) )
  {
    throw QgsProcessingException( QObject::tr( "Failed to create MBTiles file %1: %2" ).arg( outputFile, mMbtilesWriter->lastError() ) );
  }
  mMbtilesWriter->setMetadataValue( u"format"_s, mTileFormat.toLower() );
  mMbtilesWriter->setMetadataValue( u"name"_s, QFileInfo( outputFile ).baseName() );
  mMbtilesWriter->setMetadataValue( u"description"_s, QFileInfo( outputFile ).baseName() );
  mMbtilesWriter->setMetadataValue( u"version"_s, u"1.1"_s );
  mMbtilesWriter->setMetadataValue( u"type"_s, u"overlay"_s );
  mMbtilesWriter->setMetadataValue( u"minzoom"_s, QString::number( mMinZoom ) );
  mMbtilesWriter->setMetadataValue( u"maxzoom"_s, QString::number( mMaxZoom ) );
  QString boundsStr = QString( u"%1,%2,%3,%4"_s ).arg( mWgs84Extent.xMinimum() ).arg( mWgs84Extent.yMinimum() ).arg( mWgs84Extent.xMaximum() ).arg( mWgs84Extent.yMaximum() );
  mMbtilesWriter->setMetadataValue( u"bounds"_s, boundsStr );

  long long totalTiles = 0;
  mTotalMetaTiles = 0;
  for ( int z = mMinZoom; z <= mMaxZoom; z++ )
  {
    if ( feedback->isCanceled() )
      break;

    long long tileCount = 0;
    mMetaTiles += getMetatiles( mWgs84Extent, z, tileCount, mMetaTileSize );
    feedback->pushInfo( QObject::tr( "%1 metatiles (%2 tiles) will be created for zoom level %3" ).arg( mMetaTiles.size() - mTotalMetaTiles ).arg( tileCount ).arg( z ) );
    mTotalMetaTiles = mMetaTiles.size();
    totalTiles += tileCount;
  }
  if ( mTotalMetaTiles == 0 )
  {
    throw QgsProcessingException( QObject::tr( "No metatiles will be created -- please check the extent and zoom limits" ) );
  }

  feedback->pushInfo( QObject::tr( "A total of %1 metatiles (%2 tiles) will be created" ).arg( mTotalMetaTiles ).arg( totalTiles ) );

  checkLayersUsagePolicy( feedback );

  for ( QgsMapLayer *layer : std::as_const( mLayers ) )
  {
    layer->moveToThread( QThread::currentThread() );
  }

  doExport( feedback );

  qDeleteAll( mLayers );
  mLayers.clear();

  if ( !feedback->isCanceled() )
  {
    mMbtilesWriter->finalize();
  }

  if ( mSkipEmptyTiles )
  {
    feedback->pushInfo( QObject::tr( "Wrote %1 total tiles, skipped %2 empty tiles" ).arg( mTilesWritten.load() ).arg( mEmptyTiles.load() ) );
  }
  QVariantMap results;
  results.insert( u"OUTPUT_FILE"_s, outputFile );

  // try to load the result as a raster layer
  if ( !feedback->isCanceled() )
  {
    auto layer = std::make_unique<QgsRasterLayer>( outputFile, "OUTPUT_LAYER", u"gdal"_s );
    if ( !layer->isValid() )
    {
      feedback->reportError( QObject::tr( "Failed to open MBTiles file as a raster layer" ) );
    }
    const QString layerId = layer->id();
    const QgsProcessingContext::LayerDetails details( layer->name(), context.project(), u"OUTPUT_LAYER"_s, QgsProcessingUtils::LayerHint::Raster );
    details.setOutputLayerName( layer.get() );
    context.addLayerToLoadOnCompletion( layerId, details );
    context.temporaryLayerStore()->addMapLayer( layer.release() );
    results.insert( u"OUTPUT_LAYER"_s, layerId );
  }

  return results;
}

void QgsXyzTilesMbtilesAlgorithm::processMetaTile( const MetaTile &metaTile, const QImage &renderedImg, QgsProcessingFeedback *feedback )
{
  mActivePostProcessingTasks++;

  mPostProcessingPool->start( [this, feedback, metaTile, renderedImg]() {
    const bool testEmptyTilesUsingAlpha0 = mTileFormat != "JPG"_L1 && mBackgroundColor.alpha() == 0;
    long long localWritten = 0;
    long long localEmpty = 0;

    QList<QgsMbTiles::TileData> metatileTiles;
    metatileTiles.reserve( metaTile.tiles.size() );

    for ( auto it = metaTile.tiles.constBegin(); it != metaTile.tiles.constEnd(); ++it )
    {
      if ( feedback->isCanceled() )
        break;

      const QPair<int, int> tm = it.key();

      const QImage tileImage = renderedImg.copy( mTileWidth * tm.first, mTileHeight * tm.second, mTileWidth, mTileHeight );

      const bool skipTile = mSkipEmptyTiles && ( testEmptyTilesUsingAlpha0 ? QgsImageOperation::isBlankImage( tileImage ) : QgsImageOperation::isSingleColor( tileImage, mBackgroundColor ) );
      if ( skipTile )
      {
        localEmpty++;
        continue;
      }

      QByteArray bytes;
      QBuffer buffer( &bytes );
      buffer.open( QIODevice::WriteOnly );
      tileImage.save( &buffer, mTileFormat.toStdString().c_str(), mJpgQuality );

      const Tile tile = it.value();
      const int tileY = tile2tms( tile.y, tile.z );
      metatileTiles.append( { tile.z, tile.x, tileY, bytes } );
      localWritten++;
    }

    mWriteQueue->push( metatileTiles );

    mTilesWritten += localWritten;
    mEmptyTiles += localEmpty;

    mActivePostProcessingTasks--;
    checkPipelineFinished( feedback );
  } );
}

void QgsXyzTilesMbtilesAlgorithm::doExport( QgsProcessingFeedback *feedback )
{
  mPostProcessingPool = std::make_unique<QThreadPool>();
  mPostProcessingPool->setMaxThreadCount( std::max( 1, mThreadsNumber ) );

  PendingTilesToWriteQueue queue;
  mWriteQueue = &queue;

  QThread *dbThread = QThread::create( [this, &queue]() {
    QList<QgsMbTiles::TileData> batch;
    constexpr int BATCH_SIZE = 10000;
    batch.reserve( BATCH_SIZE );
    while ( queue.popBatch( batch, BATCH_SIZE ) )
    {
      mMbtilesWriter->setTileData( batch );
      batch.clear();
    }
  } );
  dbThread->start();

  QEventLoop loop;
  mEventLoop = &loop;
  mJobOwner.reset( new QObject() );

  startJobs( feedback );

  if ( !mMetaTiles.isEmpty() || !mRendererJobs.isEmpty() || mActivePostProcessingTasks.load() > 0 )
  {
    loop.exec();
  }

  for ( auto it = mRendererJobs.constBegin(); it != mRendererJobs.constEnd(); it++ )
  {
    it.key()->cancel();
    it.key()->deleteLater();
  }
  mRendererJobs.clear();

  mPostProcessingPool->waitForDone();
  mPostProcessingPool.reset();

  queue.setFinished();
  dbThread->wait();
  delete dbThread;

  mWriteQueue = nullptr;
  mEventLoop = nullptr;
}

///@endcond
