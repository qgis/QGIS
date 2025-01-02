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

#include <QBuffer>

#include "qgslayertree.h"
#include "qgslayertreelayer.h"
#include "qgsexpressioncontextutils.h"
#include "qgsmaplayerutils.h"
#include "qgsprovidermetadata.h"

///@cond PRIVATE

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

void extent2TileXY( QgsRectangle extent, const int zoom, int &xMin, int &yMin, int &xMax, int &yMax )
{
  xMin = lon2tileX( extent.xMinimum(), zoom );
  yMin = lat2tileY( extent.yMinimum(), zoom );
  xMax = lon2tileX( extent.xMaximum(), zoom );
  yMax = lat2tileY( extent.xMaximum(), zoom );
}

QList<MetaTile> getMetatiles( const QgsRectangle extent, const int zoom, const int tileSize )
{
  int minX = lon2tileX( extent.xMinimum(), zoom );
  int minY = lat2tileY( extent.yMaximum(), zoom );
  int maxX = lon2tileX( extent.xMaximum(), zoom );
  int maxY = lat2tileY( extent.yMinimum(), zoom );
  ;

  int i = 0;
  QMap<QString, MetaTile> tiles;
  for ( int x = minX; x <= maxX; x++ )
  {
    int j = 0;
    for ( int y = minY; y <= maxY; y++ )
    {
      QString key = QStringLiteral( "%1:%2" ).arg( ( int ) ( i / tileSize ) ).arg( ( int ) ( j / tileSize ) );
      MetaTile tile = tiles.value( key, MetaTile() );
      tile.addTile( i % tileSize, j % tileSize, Tile( x, y, zoom ) );
      tiles.insert( key, tile );
      j++;
    }
    i++;
  }
  return tiles.values();
}

////

QString QgsXyzTilesBaseAlgorithm::group() const
{
  return QObject::tr( "Raster tools" );
}

QString QgsXyzTilesBaseAlgorithm::groupId() const
{
  return QStringLiteral( "rastertools" );
}

Qgis::ProcessingAlgorithmFlags QgsXyzTilesBaseAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::RequiresProject;
}

void QgsXyzTilesBaseAlgorithm::createCommonParameters()
{
  addParameter( new QgsProcessingParameterExtent( QStringLiteral( "EXTENT" ), QObject::tr( "Extent" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "ZOOM_MIN" ), QObject::tr( "Minimum zoom" ), Qgis::ProcessingNumberParameterType::Integer, 12, false, 0, 25 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "ZOOM_MAX" ), QObject::tr( "Maximum zoom" ), Qgis::ProcessingNumberParameterType::Integer, 12, false, 0, 25 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "DPI" ), QObject::tr( "DPI" ), Qgis::ProcessingNumberParameterType::Integer, 96, false, 48, 600 ) );
  addParameter( new QgsProcessingParameterColor( QStringLiteral( "BACKGROUND_COLOR" ), QObject::tr( "Background color" ), QColor( Qt::transparent ), true, true ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "ANTIALIAS" ), QObject::tr( "Enable antialiasing" ), true ) );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "TILE_FORMAT" ), QObject::tr( "Tile format" ), QStringList() << QStringLiteral( "PNG" ) << QStringLiteral( "JPG" ), false, 0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "QUALITY" ), QObject::tr( "Quality (JPG only)" ), Qgis::ProcessingNumberParameterType::Integer, 75, false, 1, 100 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "METATILESIZE" ), QObject::tr( "Metatile size" ), Qgis::ProcessingNumberParameterType::Integer, 4, false, 1, 20 ) );
}

bool QgsXyzTilesBaseAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsProject *project = context.project();

  const QList<QgsLayerTreeLayer *> projectLayers = project->layerTreeRoot()->findLayers();
  QSet<QString> visibleLayers;
  for ( const QgsLayerTreeLayer *layer : projectLayers )
  {
    if ( layer->isVisible() )
    {
      visibleLayers << layer->layer()->id();
    }
  }

  QList<QgsMapLayer *> renderLayers = project->layerTreeRoot()->layerOrder();
  for ( QgsMapLayer *layer : renderLayers )
  {
    if ( visibleLayers.contains( layer->id() ) )
    {
      QgsMapLayer *clonedLayer = layer->clone();
      clonedLayer->moveToThread( nullptr );
      mLayers << clonedLayer;
    }
  }

  QgsRectangle extent = parameterAsExtent( parameters, QStringLiteral( "EXTENT" ), context );
  QgsCoordinateReferenceSystem extentCrs = parameterAsExtentCrs( parameters, QStringLiteral( "EXTENT" ), context );
  QgsCoordinateTransform ct( extentCrs, project->crs(), context.transformContext() );
  mExtent = ct.transformBoundingBox( extent );

  mMinZoom = parameterAsInt( parameters, QStringLiteral( "ZOOM_MIN" ), context );
  mMaxZoom = parameterAsInt( parameters, QStringLiteral( "ZOOM_MAX" ), context );
  mDpi = parameterAsInt( parameters, QStringLiteral( "DPI" ), context );
  mBackgroundColor = parameterAsColor( parameters, QStringLiteral( "BACKGROUND_COLOR" ), context );
  mAntialias = parameterAsBool( parameters, QStringLiteral( "ANTIALIAS" ), context );
  mTileFormat = parameterAsEnum( parameters, QStringLiteral( "TILE_FORMAT" ), context ) ? QStringLiteral( "JPG" ) : QStringLiteral( "PNG" );
  mJpgQuality = mTileFormat == QLatin1String( "JPG" ) ? parameterAsInt( parameters, QStringLiteral( "QUALITY" ), context ) : -1;
  mMetaTileSize = parameterAsInt( parameters, QStringLiteral( "METATILESIZE" ), context );
  mThreadsNumber = context.maximumThreads();
  mTransformContext = context.transformContext();
  mFeedback = feedback;

  mWgs84Crs = QgsCoordinateReferenceSystem( "EPSG:4326" );
  mMercatorCrs = QgsCoordinateReferenceSystem( "EPSG:3857" );
  mSrc2Wgs = QgsCoordinateTransform( project->crs(), mWgs84Crs, context.transformContext() );
  mWgs2Mercator = QgsCoordinateTransform( mWgs84Crs, mMercatorCrs, context.transformContext() );

  mWgs84Extent = mSrc2Wgs.transformBoundingBox( mExtent );

  if ( parameters.contains( QStringLiteral( "TILE_WIDTH" ) ) )
  {
    mTileWidth = parameterAsInt( parameters, QStringLiteral( "TILE_WIDTH" ), context );
  }

  if ( parameters.contains( QStringLiteral( "TILE_HEIGHT" ) ) )
  {
    mTileHeight = parameterAsInt( parameters, QStringLiteral( "TILE_HEIGHT" ), context );
  }

  if ( mTileFormat != QLatin1String( "PNG" ) && mBackgroundColor.alpha() != 255 )
  {
    feedback->pushWarning( QObject::tr( "Background color setting ignored, the JPG format only supports fully opaque colors" ) );
  }

  return true;
}

void QgsXyzTilesBaseAlgorithm::checkLayersUsagePolicy( QgsProcessingFeedback *feedback )
{
  if ( mTotalTiles > MAXIMUM_OPENSTREETMAP_TILES_FETCH )
  {
    for ( QgsMapLayer *layer : std::as_const( mLayers ) )
    {
      if ( QgsMapLayerUtils::isOpenStreetMapLayer( layer ) )
      {
        // Prevent bulk downloading of tiles from openstreetmap.org as per OSMF tile usage policy
        feedback->pushFormattedMessage( QObject::tr( "Layer %1 will be skipped as the algorithm leads to bulk downloading behavior which is prohibited by the %2OpenStreetMap Foundation tile usage policy%3" ).arg( layer->name(), QStringLiteral( "<a href=\"https://operations.osmfoundation.org/policies/tiles/\">" ), QStringLiteral( "</a>" ) ), QObject::tr( "Layer %1 will be skipped as the algorithm leads to bulk downloading behavior which is prohibited by the %2OpenStreetMap Foundation tile usage policy%3" ).arg( layer->name(), QString(), QString() ) );
        mLayers.removeAll( layer );
        delete layer;
      }
    }
  }
}

void QgsXyzTilesBaseAlgorithm::startJobs()
{
  while ( mRendererJobs.size() < mThreadsNumber && !mMetaTiles.empty() )
  {
    MetaTile metaTile = mMetaTiles.takeFirst();

    QgsMapSettings settings;
    settings.setExtent( mWgs2Mercator.transformBoundingBox( metaTile.extent() ) );
    settings.setOutputImageFormat( QImage::Format_ARGB32_Premultiplied );
    settings.setTransformContext( mTransformContext );
    settings.setDestinationCrs( mMercatorCrs );
    settings.setLayers( mLayers );
    settings.setOutputDpi( mDpi );
    settings.setFlag( Qgis::MapSettingsFlag::Antialiasing, mAntialias );
    if ( mTileFormat == QLatin1String( "PNG" ) || mBackgroundColor.alpha() == 255 )
    {
      settings.setBackgroundColor( mBackgroundColor );
    }
    QSize size( mTileWidth * metaTile.rows, mTileHeight * metaTile.cols );
    settings.setOutputSize( size );

    QgsLabelingEngineSettings labelingSettings = settings.labelingEngineSettings();
    labelingSettings.setFlag( Qgis::LabelingFlag::UsePartialCandidates, false );
    settings.setLabelingEngineSettings( labelingSettings );

    QgsExpressionContext exprContext = settings.expressionContext();
    exprContext.appendScope( QgsExpressionContextUtils::mapSettingsScope( settings ) );
    settings.setExpressionContext( exprContext );

    QgsMapRendererSequentialJob *job = new QgsMapRendererSequentialJob( settings );
    mRendererJobs.insert( job, metaTile );
    QObject::connect( job, &QgsMapRendererJob::finished, mFeedback, [this, job]() { processMetaTile( job ); } );
    job->start();
  }
}

// Native XYZ tiles (directory) algorithm

QString QgsXyzTilesDirectoryAlgorithm::name() const
{
  return QStringLiteral( "tilesxyzdirectory" );
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
  return QObject::tr( "Generates XYZ tiles of map canvas content and saves them as individual images in a directory." );
}

QgsXyzTilesDirectoryAlgorithm *QgsXyzTilesDirectoryAlgorithm::createInstance() const
{
  return new QgsXyzTilesDirectoryAlgorithm();
}

void QgsXyzTilesDirectoryAlgorithm::initAlgorithm( const QVariantMap & )
{
  createCommonParameters();
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "TILE_WIDTH" ), QObject::tr( "Tile width" ), Qgis::ProcessingNumberParameterType::Integer, 256, false, 1, 4096 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "TILE_HEIGHT" ), QObject::tr( "Tile height" ), Qgis::ProcessingNumberParameterType::Integer, 256, false, 1, 4096 ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "TMS_CONVENTION" ), QObject::tr( "Use inverted tile Y axis (TMS convention)" ), false ) );

  std::unique_ptr<QgsProcessingParameterString> titleParam = std::make_unique<QgsProcessingParameterString>( QStringLiteral( "HTML_TITLE" ), QObject::tr( "Leaflet HTML output title" ), QVariant(), false, true );
  titleParam->setFlags( titleParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( titleParam.release() );
  std::unique_ptr<QgsProcessingParameterString> attributionParam = std::make_unique<QgsProcessingParameterString>( QStringLiteral( "HTML_ATTRIBUTION" ), QObject::tr( "Leaflet HTML output attribution" ), QVariant(), false, true );
  attributionParam->setFlags( attributionParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( attributionParam.release() );
  std::unique_ptr<QgsProcessingParameterBoolean> osmParam = std::make_unique<QgsProcessingParameterBoolean>( QStringLiteral( "HTML_OSM" ), QObject::tr( "Include OpenStreetMap basemap in Leaflet HTML output" ), false );
  osmParam->setFlags( osmParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( osmParam.release() );

  addParameter( new QgsProcessingParameterFolderDestination( QStringLiteral( "OUTPUT_DIRECTORY" ), QObject::tr( "Output directory" ) ) );
  addParameter( new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT_HTML" ), QObject::tr( "Output html (Leaflet)" ), QObject::tr( "HTML files (*.html)" ), QVariant(), true ) );
}

QVariantMap QgsXyzTilesDirectoryAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const bool tms = parameterAsBoolean( parameters, QStringLiteral( "TMS_CONVENTION" ), context );
  const QString title = parameterAsString( parameters, QStringLiteral( "HTML_TITLE" ), context );
  const QString attribution = parameterAsString( parameters, QStringLiteral( "HTML_ATTRIBUTION" ), context );
  const bool useOsm = parameterAsBoolean( parameters, QStringLiteral( "HTML_OSM" ), context );
  QString outputDir = parameterAsString( parameters, QStringLiteral( "OUTPUT_DIRECTORY" ), context );
  const QString outputHtml = parameterAsString( parameters, QStringLiteral( "OUTPUT_HTML" ), context );

  mOutputDir = outputDir;
  mTms = tms;

  mTotalTiles = 0;
  for ( int z = mMinZoom; z <= mMaxZoom; z++ )
  {
    if ( feedback->isCanceled() )
      break;

    mMetaTiles += getMetatiles( mWgs84Extent, z, mMetaTileSize );
    feedback->pushWarning( QObject::tr( "%1 tiles will be created for zoom level %2" ).arg( mMetaTiles.size() - mTotalTiles ).arg( z ) );
    mTotalTiles = mMetaTiles.size();
  }
  feedback->pushWarning( QObject::tr( "A total of %1 tiles will be created" ).arg( mTotalTiles ) );

  checkLayersUsagePolicy( feedback );

  for ( QgsMapLayer *layer : std::as_const( mLayers ) )
  {
    layer->moveToThread( QThread::currentThread() );
  }

  QEventLoop loop;
  // cppcheck-suppress danglingLifetime
  mEventLoop = &loop;
  startJobs();
  loop.exec();

  qDeleteAll( mLayers );
  mLayers.clear();

  QVariantMap results;
  results.insert( QStringLiteral( "OUTPUT_DIRECTORY" ), outputDir );

  if ( !outputHtml.isEmpty() )
  {
    QString osm = QStringLiteral(
                    "var osm_layer = L.tileLayer('https://tile.openstreetmap.org/{z}/{x}/{y}.png',"
                    "{minZoom: %1, maxZoom: %2, attribution: '&copy; <a href=\"https://www.openstreetmap.org/copyright\">OpenStreetMap</a> contributors'}).addTo(map);"
    )
                    .arg( mMinZoom )
                    .arg( mMaxZoom );

    QString addOsm = useOsm ? osm : QString();
    QString tmsConvention = tms ? QStringLiteral( "true" ) : QStringLiteral( "false" );
    QString attr = attribution.isEmpty() ? QStringLiteral( "Created by QGIS" ) : attribution;
    QString tileSource = QStringLiteral( "'file:///%1/{z}/{x}/{y}.%2'" )
                           .arg( outputDir.replace( "\\", "/" ).toHtmlEscaped() )
                           .arg( mTileFormat.toLower() );

    QString html = QStringLiteral(
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
                     .arg( title.isEmpty() ? QStringLiteral( "Leaflet preview" ) : title )
                     .arg( mWgs84Extent.center().y() )
                     .arg( mWgs84Extent.center().x() )
                     .arg( ( mMaxZoom + mMinZoom ) / 2 )
                     .arg( addOsm )
                     .arg( tileSource )
                     .arg( mMinZoom )
                     .arg( mMaxZoom )
                     .arg( tmsConvention )
                     .arg( attr );

    QFile htmlFile( outputHtml );
    if ( !htmlFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    {
      throw QgsProcessingException( QObject::tr( "Could not open html file %1" ).arg( outputHtml ) );
    }
    QTextStream fout( &htmlFile );
#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
    fout.setCodec( "UTF-8" );
#endif
    fout << html;

    results.insert( QStringLiteral( "OUTPUT_HTML" ), outputHtml );
  }

  return results;
}

void QgsXyzTilesDirectoryAlgorithm::processMetaTile( QgsMapRendererSequentialJob *job )
{
  MetaTile metaTile = mRendererJobs.value( job );
  QImage img = job->renderedImage();

  QMap<QPair<int, int>, Tile>::const_iterator it = metaTile.tiles.constBegin();
  while ( it != metaTile.tiles.constEnd() )
  {
    QPair<int, int> tm = it.key();
    Tile tile = it.value();
    QImage tileImage = img.copy( mTileWidth * tm.first, mTileHeight * tm.second, mTileWidth, mTileHeight );
    QDir tileDir( QStringLiteral( "%1/%2/%3" ).arg( mOutputDir ).arg( tile.z ).arg( tile.x ) );
    tileDir.mkpath( tileDir.absolutePath() );
    int y = tile.y;
    if ( mTms )
    {
      y = tile2tms( y, tile.z );
    }
    tileImage.save( QStringLiteral( "%1/%2.%3" ).arg( tileDir.absolutePath() ).arg( y ).arg( mTileFormat.toLower() ), mTileFormat.toStdString().c_str(), mJpgQuality );
    ++it;
  }

  mRendererJobs.remove( job );
  job->deleteLater();

  mFeedback->setProgress( 100.0 * ( mProcessedTiles++ ) / mTotalTiles );

  if ( mFeedback->isCanceled() )
  {
    while ( mRendererJobs.size() > 0 )
    {
      QgsMapRendererSequentialJob *j = mRendererJobs.firstKey();
      j->cancel();
      mRendererJobs.remove( j );
      j->deleteLater();
    }
    mRendererJobs.clear();
    if ( mEventLoop )
    {
      mEventLoop->exit();
    }
    return;
  }

  if ( mMetaTiles.size() > 0 )
  {
    startJobs();
  }
  else if ( mMetaTiles.size() == 0 && mRendererJobs.size() == 0 )
  {
    if ( mEventLoop )
    {
      mEventLoop->exit();
    }
  }
}

// Native XYZ tiles (MBTiles) algorithm

QString QgsXyzTilesMbtilesAlgorithm::name() const
{
  return QStringLiteral( "tilesxyzmbtiles" );
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
  return QObject::tr( "Generates XYZ tiles of map canvas content and saves them as an MBTiles file." );
}

QgsXyzTilesMbtilesAlgorithm *QgsXyzTilesMbtilesAlgorithm::createInstance() const
{
  return new QgsXyzTilesMbtilesAlgorithm();
}

void QgsXyzTilesMbtilesAlgorithm::initAlgorithm( const QVariantMap & )
{
  createCommonParameters();
  addParameter( new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT_FILE" ), QObject::tr( "Output" ), QObject::tr( "MBTiles files (*.mbtiles *.MBTILES)" ) ) );
}

QVariantMap QgsXyzTilesMbtilesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString outputFile = parameterAsString( parameters, QStringLiteral( "OUTPUT_FILE" ), context );

  mMbtilesWriter = std::make_unique<QgsMbTiles>( outputFile );
  if ( !mMbtilesWriter->create() )
  {
    throw QgsProcessingException( QObject::tr( "Failed to create MBTiles file %1" ).arg( outputFile ) );
  }
  mMbtilesWriter->setMetadataValue( "format", mTileFormat.toLower() );
  mMbtilesWriter->setMetadataValue( "name", QFileInfo( outputFile ).baseName() );
  mMbtilesWriter->setMetadataValue( "description", QFileInfo( outputFile ).baseName() );
  mMbtilesWriter->setMetadataValue( "version", QStringLiteral( "1.1" ) );
  mMbtilesWriter->setMetadataValue( "type", QStringLiteral( "overlay" ) );
  mMbtilesWriter->setMetadataValue( "minzoom", QString::number( mMinZoom ) );
  mMbtilesWriter->setMetadataValue( "maxzoom", QString::number( mMaxZoom ) );
  QString boundsStr = QString( "%1,%2,%3,%4" )
                        .arg( mWgs84Extent.xMinimum() )
                        .arg( mWgs84Extent.yMinimum() )
                        .arg( mWgs84Extent.xMaximum() )
                        .arg( mWgs84Extent.yMaximum() );
  mMbtilesWriter->setMetadataValue( "bounds", boundsStr );

  mTotalTiles = 0;
  for ( int z = mMinZoom; z <= mMaxZoom; z++ )
  {
    if ( feedback->isCanceled() )
      break;

    mMetaTiles += getMetatiles( mWgs84Extent, z, mMetaTileSize );
    feedback->pushInfo( QObject::tr( "%1 tiles will be created for zoom level %2" ).arg( mMetaTiles.size() - mTotalTiles ).arg( z ) );
    mTotalTiles = mMetaTiles.size();
  }
  feedback->pushInfo( QObject::tr( "A total of %1 tiles will be created" ).arg( mTotalTiles ) );

  checkLayersUsagePolicy( feedback );

  for ( QgsMapLayer *layer : std::as_const( mLayers ) )
  {
    layer->moveToThread( QThread::currentThread() );
  }

  QEventLoop loop;
  // cppcheck-suppress danglingLifetime
  mEventLoop = &loop;
  startJobs();
  loop.exec();

  qDeleteAll( mLayers );
  mLayers.clear();

  QVariantMap results;
  results.insert( QStringLiteral( "OUTPUT_FILE" ), outputFile );
  return results;
}

void QgsXyzTilesMbtilesAlgorithm::processMetaTile( QgsMapRendererSequentialJob *job )
{
  MetaTile metaTile = mRendererJobs.value( job );
  QImage img = job->renderedImage();

  QMap<QPair<int, int>, Tile>::const_iterator it = metaTile.tiles.constBegin();
  while ( it != metaTile.tiles.constEnd() )
  {
    QPair<int, int> tm = it.key();
    Tile tile = it.value();
    QImage tileImage = img.copy( mTileWidth * tm.first, mTileHeight * tm.second, mTileWidth, mTileHeight );
    QByteArray ba;
    QBuffer buffer( &ba );
    buffer.open( QIODevice::WriteOnly );
    tileImage.save( &buffer, mTileFormat.toStdString().c_str(), mJpgQuality );
    mMbtilesWriter->setTileData( tile.z, tile.x, tile2tms( tile.y, tile.z ), ba );
    ++it;
  }

  mRendererJobs.remove( job );
  job->deleteLater();

  mFeedback->setProgress( 100.0 * ( mProcessedTiles++ ) / mTotalTiles );

  if ( mFeedback->isCanceled() )
  {
    while ( mRendererJobs.size() > 0 )
    {
      QgsMapRendererSequentialJob *j = mRendererJobs.firstKey();
      j->cancel();
      mRendererJobs.remove( j );
      j->deleteLater();
    }
    mRendererJobs.clear();
    if ( mEventLoop )
    {
      mEventLoop->exit();
    }
    return;
  }

  if ( mMetaTiles.size() > 0 )
  {
    startJobs();
  }
  else if ( mMetaTiles.size() == 0 && mRendererJobs.size() == 0 )
  {
    if ( mEventLoop )
    {
      mEventLoop->exit();
    }
  }
}

///@endcond
