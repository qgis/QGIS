/***************************************************************************
                         qgsalgorithmdownloadvectortiles.cpp
                         ---------------------
    begin                : May 2023
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

#include "qgsalgorithmdownloadvectortiles.h"

#include "qgsvectortilelayer.h"
#include "qgsvectortiledataprovider.h"
#include "qgsziputils.h"

///@cond PRIVATE

class SetStylePostProcessor : public QgsProcessingLayerPostProcessorInterface
{
  public:

    SetStylePostProcessor( QgsVectorTileLayer *inputLayer )
      : mLayer( inputLayer )
    {}

    void postProcessLayer( QgsMapLayer *layer, QgsProcessingContext &, QgsProcessingFeedback * ) override
    {
      if ( QgsVectorTileLayer *tileLayer = qobject_cast< QgsVectorTileLayer * >( layer ) )
      {
        QDomDocument doc( QStringLiteral( "qgis" ) );
        QString errorMsg;
        mLayer->exportNamedStyle( doc, errorMsg );
        tileLayer->importNamedStyle( doc, errorMsg );
        tileLayer->triggerRepaint();
      }
    }

  private:

    QgsVectorTileLayer *mLayer;
};

QString QgsDownloadVectorTilesAlgorithm::name() const
{
  return QStringLiteral( "downloadvectortiles" );
}

QString QgsDownloadVectorTilesAlgorithm::displayName() const
{
  return QObject::tr( "Download vector tiles" );
}

QStringList QgsDownloadVectorTilesAlgorithm::tags() const
{
  return QObject::tr( "vector,split,field,unique" ).split( ',' );
}

QString QgsDownloadVectorTilesAlgorithm::group() const
{
  return QObject::tr( "Vector tiles" );
}

QString QgsDownloadVectorTilesAlgorithm::groupId() const
{
  return QStringLiteral( "vectortiles" );
}

QString QgsDownloadVectorTilesAlgorithm::shortHelpString() const
{
  return QObject::tr( "Downloads vector tiles of the input vector tile layer and saves them in the local vector tile file." );
}

QgsDownloadVectorTilesAlgorithm *QgsDownloadVectorTilesAlgorithm::createInstance() const
{
  return new QgsDownloadVectorTilesAlgorithm();
}

void QgsDownloadVectorTilesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMapLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ), QVariant(), false, QList<int>() << QgsProcessing::TypeVectorTile ) );
  addParameter( new QgsProcessingParameterExtent( QStringLiteral( "EXTENT" ), QObject::tr( "Extent" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "MAX_ZOOM" ), QObject::tr( "Maximum zoom level to download" ), QgsProcessingParameterNumber::Integer, 10, false, 0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "TILE_LIMIT" ), QObject::tr( "Tile limit" ), QgsProcessingParameterNumber::Integer, 100, false, 0 ) );
  addParameter( new QgsProcessingParameterVectorTileDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Output" ) ) );
}

QVariantMap QgsDownloadVectorTilesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsMapLayer *layer = parameterAsLayer( parameters, QStringLiteral( "INPUT" ), context );
  if ( !layer )
    throw QgsProcessingException( QObject::tr( "Invalid input layer" ) );

  const QgsRectangle extent = parameterAsExtent( parameters, QStringLiteral( "EXTENT" ), context, layer->crs() );
  int maxZoomLevel = parameterAsInt( parameters, QStringLiteral( "MAX_ZOOM" ), context );
  const long long tileLimit = static_cast< long long >( parameterAsInt( parameters, QStringLiteral( "TILE_LIMIT" ), context ) );
  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );

  QgsVectorTileLayer *vtLayer = qobject_cast< QgsVectorTileLayer * >( layer );
  const QgsVectorTileDataProvider *provider = qgis::down_cast< const QgsVectorTileDataProvider * >( vtLayer->dataProvider() );
  QgsVectorTileMatrixSet tileMatrixSet = vtLayer->tileMatrixSet();

  if ( maxZoomLevel > vtLayer->sourceMaxZoom() )
  {
    throw QgsProcessingException( QObject::tr( "Requested maximum zoom level is bigger than available zoom level in the source layer. Please, select zoom level lower or equal to %1." ).arg( vtLayer->sourceMaxZoom() ) );
  }

  // count total number of tiles in the requested extent and zoom levels to see if it exceeds the tile limit
  long long tileCount = 0;
  QMap<int, QgsTileRange> tileRanges;
  for ( int i = 0; i <= maxZoomLevel; i++ )
  {
    QgsTileMatrix tileMatrix = tileMatrixSet.tileMatrix( i );
    QgsTileRange tileRange = tileMatrix.tileRangeFromExtent( extent );
    tileRanges.insert( i, tileRange );
    tileCount += static_cast< long long >( ( tileRange.endColumn() - tileRange.startColumn() + 1 ) * ( tileRange.endRow() - tileRange.startRow() + 1 ) );
  }
  if ( tileCount > tileLimit )
  {
    throw QgsProcessingException( QObject::tr( "Requested number of tiles %1 exceeds limit of %2 tiles. Please, select a smaller extent, reduce maximum zoom level or increase tile limit." ).arg( tileCount ).arg( tileLimit ) );
  }

  std::unique_ptr<QgsMbTiles> writer = std::make_unique<QgsMbTiles>( outputFile );
  if ( !writer->create() )
  {
    throw QgsProcessingException( QObject::tr( "Failed to create MBTiles file %1" ).arg( outputFile ) );
  }
  writer->setMetadataValue( "format", "pbf" );
  writer->setMetadataValue( "name",  vtLayer->name() );
  writer->setMetadataValue( "minzoom", QString::number( vtLayer->sourceMinZoom() ) );
  writer->setMetadataValue( "maxzoom", QString::number( maxZoomLevel ) );
  writer->setMetadataValue( "crs",  tileMatrixSet.rootMatrix().crs().authid() );
  try
  {
    QgsCoordinateTransform ct( tileMatrixSet.rootMatrix().crs(), QgsCoordinateReferenceSystem( "EPSG:4326" ), context.transformContext() );
    ct.setBallparkTransformsAreAppropriate( true );
    QgsRectangle wgsExtent = ct.transformBoundingBox( extent );
    QString boundsStr = QString( "%1,%2,%3,%4" )
                        .arg( wgsExtent.xMinimum() ).arg( wgsExtent.yMinimum() )
                        .arg( wgsExtent.xMaximum() ).arg( wgsExtent.yMaximum() );
    writer->setMetadataValue( "bounds", boundsStr );
  }
  catch ( const QgsCsException & )
  {
    // bounds won't be written (not a problem - it is an optional value)
  }

  QgsProcessingMultiStepFeedback multiStepFeedback( maxZoomLevel + 1, feedback );

  std::unique_ptr<QgsVectorTileLoader> loader;
  QList<QgsVectorTileRawData> rawTiles;

  QMap<int, QgsTileRange>::const_iterator it = tileRanges.constBegin();
  while ( it != tileRanges.constEnd() )
  {
    if ( feedback->isCanceled() )
      break;

    multiStepFeedback.setCurrentStep( it.key() );

    QgsTileMatrix tileMatrix = tileMatrixSet.tileMatrix( it.key() );
    tileCount = static_cast< long long >( ( it.value().endColumn() - it.value().startColumn() + 1 ) * ( it.value().endRow() - it.value().startRow() + 1 ) );

    const QPointF viewCenter = tileMatrix.mapToTileCoordinates( extent.center() );

    long long tileNumber = 0;
    if ( !provider->supportsAsync() )
    {
      rawTiles = QgsVectorTileLoader::blockingFetchTileRawData( provider, tileMatrixSet, viewCenter, it.value(), it.key(), &multiStepFeedback );
      for ( const QgsVectorTileRawData &rawTile : std::as_const( rawTiles ) )
      {
        if ( feedback->isCanceled() )
          break;

        writeTile( writer.get(), rawTile );
        multiStepFeedback.setProgress( 100.0 * ( tileNumber++ ) / tileCount );
      }
    }
    else
    {
      loader.reset( new QgsVectorTileLoader( provider, tileMatrixSet, it.value(), it.key(), viewCenter, &multiStepFeedback, Qgis::RendererUsage::Export ) );
      QObject::connect( loader.get(), &QgsVectorTileLoader::tileRequestFinished, loader.get(), [ this, &writer, &multiStepFeedback, &tileNumber, &tileCount ]( const QgsVectorTileRawData & rawTile )
      {
        writeTile( writer.get(), rawTile );
        multiStepFeedback.setProgress( 100.0 * ( tileNumber++ ) / tileCount );
      } );
      loader->downloadBlocking();
    }

    ++it;
  }

  if ( context.willLoadLayerOnCompletion( outputFile ) )
  {
    context.layerToLoadOnCompletionDetails( outputFile ).setPostProcessor( new SetStylePostProcessor( vtLayer ) );
  }

  QVariantMap results;
  results.insert( QStringLiteral( "OUTPUT" ), outputFile );
  return results;
}

void QgsDownloadVectorTilesAlgorithm::writeTile( QgsMbTiles *writer, const QgsVectorTileRawData &rawTile )
{
  if ( !rawTile.data.isEmpty() )
  {
    QByteArray gzipTileData;
    QgsZipUtils::encodeGzip( rawTile.data, gzipTileData );
    int rowTMS = pow( 2, rawTile.id.zoomLevel() ) - rawTile.id.row() - 1;
    writer->setTileData( rawTile.id.zoomLevel(), rawTile.id.column(), rowTMS, gzipTileData );
  }
}

///@endcond
