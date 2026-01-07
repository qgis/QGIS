/***************************************************************************
  qgsalgorithmrasterize.cpp - QgsRasterizeAlgorithm

 ---------------------
 Original implementation in Python:

 begin                : 2016-10-05
 copyright            : (C) 2016 by OPENGIS.ch
 email                : matthias@opengis.ch

 C++ port:

 begin                : 20.11.2019
 copyright            : (C) 2019 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmrasterize.h"

#include <gdal.h>

#include "qgsgdalutils.h"
#include "qgslayertree.h"
#include "qgsmaplayerutils.h"
#include "qgsmaprenderercustompainterjob.h"
#include "qgsmapthemecollection.h"
#include "qgsprocessingparameters.h"
#include "qgsprovidermetadata.h"
#include "qgsrasterfilewriter.h"

#include <QtConcurrent>

///@cond PRIVATE

QString QgsRasterizeAlgorithm::name() const
{
  return u"rasterize"_s;
}

QString QgsRasterizeAlgorithm::displayName() const
{
  return QObject::tr( "Convert map to raster" );
}

QStringList QgsRasterizeAlgorithm::tags() const
{
  return QObject::tr( "layer,raster,convert,file,map themes,tiles,render" ).split( ',' );
}

Qgis::ProcessingAlgorithmFlags QgsRasterizeAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::RequiresProject;
}

QString QgsRasterizeAlgorithm::group() const
{
  return QObject::tr( "Raster tools" );
}

QString QgsRasterizeAlgorithm::groupId() const
{
  return u"rastertools"_s;
}

void QgsRasterizeAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterExtent(
    u"EXTENT"_s,
    QObject::tr( "Minimum extent to render" )
  ) );
  addParameter( new QgsProcessingParameterNumber(
    u"EXTENT_BUFFER"_s,
    QObject::tr( "Buffer around tiles in map units" ),
    Qgis::ProcessingNumberParameterType::Double,
    0,
    true,
    0
  ) );
  addParameter( new QgsProcessingParameterNumber(
    u"TILE_SIZE"_s,
    QObject::tr( "Tile size" ),
    Qgis::ProcessingNumberParameterType::Integer,
    1024,
    false,
    64
  ) );
  addParameter( new QgsProcessingParameterNumber(
    u"MAP_UNITS_PER_PIXEL"_s,
    QObject::tr( "Map units per pixel" ),
    Qgis::ProcessingNumberParameterType::Double,
    100,
    true,
    0
  ) );
  addParameter( new QgsProcessingParameterBoolean(
    u"MAKE_BACKGROUND_TRANSPARENT"_s,
    QObject::tr( "Make background transparent" ),
    false
  ) );

  addParameter( new QgsProcessingParameterMapTheme(
    u"MAP_THEME"_s,
    QObject::tr( "Map theme to render" ),
    QVariant(), true
  ) );

  addParameter( new QgsProcessingParameterMultipleLayers(
    u"LAYERS"_s,
    QObject::tr( "Layers to render" ),
    Qgis::ProcessingSourceType::MapLayer,
    QVariant(),
    true
  ) );
  addParameter( new QgsProcessingParameterRasterDestination(
    u"OUTPUT"_s,
    QObject::tr( "Output layer" )
  ) );
}

QString QgsRasterizeAlgorithm::shortDescription() const
{
  return QObject::tr( "Renders the map canvas to a raster file." );
}

QString QgsRasterizeAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm rasterizes map canvas content.\n\n"
                      "A map theme can be selected to render a predetermined set of layers with a defined style for each layer. "
                      "Alternatively, a set of layers can be selected if no map theme is set. "
                      "If neither map theme nor layer is set, all the visible layers in the set extent will be rendered.\n\n"
                      "The minimum extent entered will internally be extended to a multiple of the tile size." );
}

QgsRasterizeAlgorithm *QgsRasterizeAlgorithm::createInstance() const
{
  return new QgsRasterizeAlgorithm();
}


QVariantMap QgsRasterizeAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  // Note: MAP_THEME and LAYERS are handled and cloned in prepareAlgorithm
  const QgsRectangle extent { parameterAsExtent( parameters, u"EXTENT"_s, context, mCrs ) };
  const int tileSize { parameterAsInt( parameters, u"TILE_SIZE"_s, context ) };
  if ( tileSize <= 0 )
  {
    throw QgsProcessingException( QObject::tr( "Tile size must be > 0" ) );
  }
  const bool transparent { parameterAsBool( parameters, u"MAKE_BACKGROUND_TRANSPARENT"_s, context ) };
  const double mapUnitsPerPixel { parameterAsDouble( parameters, u"MAP_UNITS_PER_PIXEL"_s, context ) };
  if ( mapUnitsPerPixel <= 0 )
  {
    throw QgsProcessingException( QObject::tr( "Map units per pixel must be > 0" ) );
  }
  const double extentBuffer { parameterAsDouble( parameters, u"EXTENT_BUFFER"_s, context ) };
  const QString outputLayerFileName { parameterAsOutputLayer( parameters, u"OUTPUT"_s, context ) };

  int xTileCount { static_cast<int>( ceil( extent.width() / mapUnitsPerPixel / tileSize ) ) };
  int yTileCount { static_cast<int>( ceil( extent.height() / mapUnitsPerPixel / tileSize ) ) };
  int width { xTileCount * tileSize };
  int height { yTileCount * tileSize };
  int nBands { transparent ? 4 : 3 };

  int64_t totalTiles = 0;
  for ( auto &layer : std::as_const( mMapLayers ) )
  {
    if ( QgsMapLayerUtils::isOpenStreetMapLayer( layer.get() ) )
    {
      if ( QgsRasterLayer *rasterLayer = qobject_cast<QgsRasterLayer *>( ( layer.get() ) ) )
      {
        const QList<double> resolutions = rasterLayer->dataProvider()->nativeResolutions();
        if ( resolutions.isEmpty() )
        {
          continue;
        }

        if ( totalTiles == 0 )
        {
          const QgsCoordinateTransform ct( mCrs, rasterLayer->crs(), context.transformContext() );
          QgsRectangle extentLayer;
          try
          {
            extentLayer = ct.transform( extent );
          }
          catch ( QgsCsException & )
          {
            totalTiles = -1;
            continue;
          }

          const double mapUnitsPerPixelLayer = extentLayer.width() / width;
          int i;
          for ( i = 0; i < resolutions.size() && resolutions.at( i ) < mapUnitsPerPixelLayer; i++ )
          {
          }

          if ( i == resolutions.size() || ( i > 0 && resolutions.at( i ) - mapUnitsPerPixelLayer > mapUnitsPerPixelLayer - resolutions.at( i - 1 ) ) )
          {
            i--;
          }

          const int nbTilesWidth = std::ceil( extentLayer.width() / resolutions.at( i ) / 256 );
          const int nbTilesHeight = std::ceil( extentLayer.height() / resolutions.at( i ) / 256 );
          totalTiles = static_cast<int64_t>( nbTilesWidth ) * nbTilesHeight;
        }
        feedback->pushInfo( u"%1"_s.arg( totalTiles ) );

        if ( totalTiles > MAXIMUM_OPENSTREETMAP_TILES_FETCH )
        {
          // Prevent bulk downloading of tiles from openstreetmap.org as per OSMF tile usage policy
          feedback->pushFormattedMessage( QObject::tr( "Layer %1 will be skipped as the algorithm leads to bulk downloading behavior which is prohibited by the %2OpenStreetMap Foundation tile usage policy%3" ).arg( rasterLayer->name(), u"<a href=\"https://operations.osmfoundation.org/policies/tiles/\">"_s, u"</a>"_s ), QObject::tr( "Layer %1 will be skipped as the algorithm leads to bulk downloading behavior which is prohibited by the %2OpenStreetMap Foundation tile usage policy%3" ).arg( rasterLayer->name(), QString(), QString() ) );

          layer->deleteLater();
          std::vector<std::unique_ptr<QgsMapLayer>>::iterator position = std::find( mMapLayers.begin(), mMapLayers.end(), layer );
          if ( position != mMapLayers.end() )
          {
            mMapLayers.erase( position );
          }
        }
      }
    }
  }

  const QString driverName = parameterAsOutputRasterFormat( parameters, u"OUTPUT"_s, context );
  if ( driverName.isEmpty() )
  {
    throw QgsProcessingException( QObject::tr( "Invalid output raster format" ) );
  }

  GDALDriverH hOutputFileDriver = GDALGetDriverByName( driverName.toLocal8Bit().constData() );
  if ( !hOutputFileDriver )
  {
    throw QgsProcessingException( QObject::tr( "Error creating GDAL driver" ) );
  }

  gdal::dataset_unique_ptr hOutputDataset( GDALCreate( hOutputFileDriver, outputLayerFileName.toUtf8().constData(), width, height, nBands, GDALDataType::GDT_Byte, nullptr ) );
  if ( !hOutputDataset )
  {
    throw QgsProcessingException( QObject::tr( "Error creating GDAL output layer" ) );
  }

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION( 3, 13, 0 )
  const bool closeReportsProgress = GDALDatasetGetCloseReportsProgress( hOutputDataset.get() );
  const double maxProgressDuringBlockWriting = closeReportsProgress ? 50.0 : 100.0;
#else
  constexpr double maxProgressDuringBlockWriting = 100.0;
#endif

  GDALSetProjection( hOutputDataset.get(), mCrs.toWkt( Qgis::CrsWktVariant::PreferredGdal ).toLatin1().constData() );
  double geoTransform[6];
  geoTransform[0] = extent.xMinimum();
  geoTransform[1] = mapUnitsPerPixel;
  geoTransform[2] = 0;
  geoTransform[3] = extent.yMaximum();
  geoTransform[4] = 0;
  geoTransform[5] = -mapUnitsPerPixel;
  GDALSetGeoTransform( hOutputDataset.get(), geoTransform );

  mMapSettings.setOutputImageFormat( QImage::Format_ARGB32 );
  mMapSettings.setDestinationCrs( mCrs );
  mMapSettings.setFlag( Qgis::MapSettingsFlag::Antialiasing, true );
  mMapSettings.setFlag( Qgis::MapSettingsFlag::HighQualityImageTransforms, true );
  mMapSettings.setFlag( Qgis::MapSettingsFlag::RenderMapTile, true );
  mMapSettings.setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy::Default );
  mMapSettings.setTransformContext( context.transformContext() );
  mMapSettings.setExtentBuffer( extentBuffer );

  // Set layers cloned in prepareAlgorithm
  QList<QgsMapLayer *> layers;
  for ( const auto &lptr : mMapLayers )
  {
    layers.push_back( lptr.get() );
  }
  mMapSettings.setLayers( layers );
  mMapSettings.setLayerStyleOverrides( mMapThemeStyleOverrides );

  // Start rendering
  const double extentRatio { mapUnitsPerPixel * tileSize };
  const int numTiles { xTileCount * yTileCount };

  // Custom deleter for CPL allocation
  struct CPLDelete
  {
      void operator()( uint8_t *ptr ) const
      {
        CPLFree( ptr );
      }
  };

  QAtomicInt rendered = 0;
  QMutex rasterWriteLocker;

  const auto renderJob = [&]( const int x, const int y, QgsMapSettings mapSettings ) {
    QImage image { tileSize, tileSize, QImage::Format::Format_ARGB32 };
    mapSettings.setOutputDpi( image.logicalDpiX() );
    mapSettings.setOutputSize( image.size() );
    QPainter painter { &image };
    if ( feedback->isCanceled() )
    {
      return;
    }
    image.fill( transparent ? mapSettings.backgroundColor().rgba() : mapSettings.backgroundColor().rgb() );
    mapSettings.setExtent( QgsRectangle(
      extent.xMinimum() + x * extentRatio,
      extent.yMaximum() - ( y + 1 ) * extentRatio,
      extent.xMinimum() + ( x + 1 ) * extentRatio,
      extent.yMaximum() - y * extentRatio
    ) );
    QgsMapRendererCustomPainterJob job( mapSettings, &painter );
    job.start();
    job.waitForFinished();

    gdal::dataset_unique_ptr hIntermediateDataset( QgsGdalUtils::imageToMemoryDataset( image ) );
    if ( !hIntermediateDataset )
    {
      throw QgsProcessingException( QObject::tr( "Error reading tiles from the temporary image" ) );
    }

    const int xOffset { x * tileSize };
    const int yOffset { y * tileSize };

    std::unique_ptr<uint8_t, CPLDelete> buffer( static_cast<uint8_t *>( CPLMalloc( sizeof( uint8_t ) * static_cast<size_t>( tileSize * tileSize * nBands ) ) ) );
    CPLErr err = GDALDatasetRasterIO( hIntermediateDataset.get(), GF_Read, 0, 0, tileSize, tileSize, buffer.get(), tileSize, tileSize, GDT_Byte, nBands, nullptr, 0, 0, 0 );
    if ( err != CE_None )
    {
      throw QgsProcessingException( QObject::tr( "Error reading intermediate raster" ) );
    }

    {
      QMutexLocker locker( &rasterWriteLocker );
      err = GDALDatasetRasterIO( hOutputDataset.get(), GF_Write, xOffset, yOffset, tileSize, tileSize, buffer.get(), tileSize, tileSize, GDT_Byte, nBands, nullptr, 0, 0, 0 );
      rendered++;
      feedback->setProgress( static_cast<double>( rendered ) / numTiles * maxProgressDuringBlockWriting );
    }
    if ( err != CE_None )
    {
      throw QgsProcessingException( QObject::tr( "Error writing output raster" ) );
    }
  };

  feedback->setProgress( 0 );

  std::vector<QFuture<void>> futures;

  for ( int x = 0; x < xTileCount; ++x )
  {
    for ( int y = 0; y < yTileCount; ++y )
    {
      if ( feedback->isCanceled() )
      {
        return {};
      }
      futures.push_back( QtConcurrent::run( renderJob, x, y, mMapSettings ) );
    }
  }

  for ( auto &f : futures )
  {
    f.waitForFinished();
  }

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION( 3, 13, 0 )
  if ( closeReportsProgress )
  {
    QgsGdalProgressAdapter progress( feedback, maxProgressDuringBlockWriting );
    if ( GDALDatasetRunCloseWithoutDestroyingEx(
           hOutputDataset.get(), QgsGdalProgressAdapter::progressCallback, &progress
         )
         != CE_None )
    {
      if ( feedback->isCanceled() )
        return {};
      throw QgsProcessingException( QObject::tr( "Error writing output raster" ) );
    }
  }
#endif

  return { { u"OUTPUT"_s, outputLayerFileName } };
}


bool QgsRasterizeAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback )
  // Retrieve and clone layers
  const QString mapTheme { parameterAsString( parameters, u"MAP_THEME"_s, context ) };
  const QList<QgsMapLayer *> mapLayers { parameterAsLayerList( parameters, u"LAYERS"_s, context ) };
  if ( !mapTheme.isEmpty() && context.project()->mapThemeCollection()->hasMapTheme( mapTheme ) )
  {
    const auto constLayers { context.project()->mapThemeCollection()->mapThemeVisibleLayers( mapTheme ) };
    for ( const QgsMapLayer *ml : constLayers )
    {
      mMapLayers.push_back( std::unique_ptr<QgsMapLayer>( ml->clone() ) );
    }
    mMapThemeStyleOverrides = context.project()->mapThemeCollection()->mapThemeStyleOverrides( mapTheme );
  }
  else if ( !mapLayers.isEmpty() )
  {
    for ( const QgsMapLayer *ml : std::as_const( mapLayers ) )
    {
      mMapLayers.push_back( std::unique_ptr<QgsMapLayer>( ml->clone() ) );
    }
  }
  // Still no layers? Get them all from the project
  if ( mMapLayers.size() == 0 )
  {
    QList<QgsMapLayer *> layers;
    QgsLayerTree *root = context.project()->layerTreeRoot();
    for ( QgsLayerTreeLayer *nodeLayer : root->findLayers() )
    {
      QgsMapLayer *layer = nodeLayer->layer();
      if ( nodeLayer->isVisible() && root->layerOrder().contains( layer ) )
        layers << layer;
    }

    for ( const QgsMapLayer *ml : std::as_const( layers ) )
    {
      mMapLayers.push_back( std::unique_ptr<QgsMapLayer>( ml->clone() ) );
    }
  }

  mCrs = context.project()->crs();

  int red = context.project()->readNumEntry( u"Gui"_s, "/CanvasColorRedPart", 255 );
  int green = context.project()->readNumEntry( u"Gui"_s, "/CanvasColorGreenPart", 255 );
  int blue = context.project()->readNumEntry( u"Gui"_s, "/CanvasColorBluePart", 255 );

  const bool transparent { parameterAsBool( parameters, u"MAKE_BACKGROUND_TRANSPARENT"_s, context ) };
  QColor bgColor;
  if ( transparent )
  {
    bgColor = QColor( red, green, blue, 0 );
  }
  else
  {
    bgColor = QColor( red, green, blue );
  }
  mMapSettings.setBackgroundColor( bgColor );

  mMapSettings.setScaleMethod( context.project()->scaleMethod() );

  return mMapLayers.size() > 0;
}


///@endcond
