/***************************************************************************
  qgsalgorithmwritevectortiles.h
  ---------------------
  Date                 : April 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmwritevectortiles.h"

#include "qgsprocessingparametervectortilewriterlayers.h"
#include "qgsvectorlayer.h"
#include "qgsvectortilewriter.h"
#include <QUrl>

///@cond PRIVATE


QString QgsWriteVectorTilesBaseAlgorithm::group() const
{
  return QObject::tr( "Vector tiles" );
}

QString QgsWriteVectorTilesBaseAlgorithm::groupId() const
{
  return QStringLiteral( "vectortiles" );
}

QString QgsWriteVectorTilesBaseAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm exports one or more vector layers to vector tiles - a data format optimized for fast map rendering and small data size." );
}

void QgsWriteVectorTilesBaseAlgorithm::addBaseParameters()
{
  addParameter( new QgsProcessingParameterVectorTileWriterLayers( QStringLiteral( "LAYERS" ), QObject::tr( "Input layers" ) ) );

  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "MIN_ZOOM" ), QObject::tr( "Minimum zoom level" ), Qgis::ProcessingNumberParameterType::Integer, 0, false, 0, 24 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "MAX_ZOOM" ), QObject::tr( "Maximum zoom level" ), Qgis::ProcessingNumberParameterType::Integer, 3, false, 0, 24 ) );

  // optional extent
  addParameter( new QgsProcessingParameterExtent( QStringLiteral( "EXTENT" ), QObject::tr( "Extent" ), QVariant(), true ) );
}

QVariantMap QgsWriteVectorTilesBaseAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const int minZoom = parameterAsInt( parameters, QStringLiteral( "MIN_ZOOM" ), context );
  const int maxZoom = parameterAsInt( parameters, QStringLiteral( "MAX_ZOOM" ), context );

  const QVariant layersVariant = parameters.value( parameterDefinition( QStringLiteral( "LAYERS" ) )->name() );
  const QList<QgsVectorTileWriter::Layer> layers = QgsProcessingParameterVectorTileWriterLayers::parameterAsLayers( layersVariant, context );

  for ( const QgsVectorTileWriter::Layer &layer : layers )
  {
    if ( !layer.layer() )
      throw QgsProcessingException( QObject::tr( "Unknown input layer" ) );
  }

  QgsVectorTileWriter writer;
  QVariantMap outputs;
  prepareWriter( writer, parameters, context, outputs );

  writer.setMinZoom( minZoom );
  writer.setMaxZoom( maxZoom );
  writer.setLayers( layers );
  writer.setTransformContext( context.transformContext() );

  if ( parameters.contains( QStringLiteral( "EXTENT" ) ) )
  {
    const QgsRectangle extent = parameterAsExtent( parameters, QStringLiteral( "EXTENT" ), context, QgsCoordinateReferenceSystem( "EPSG:3857" ) );
    writer.setExtent( extent );
  }

  const bool res = writer.writeTiles( feedback );

  if ( !res )
    throw QgsProcessingException( QObject::tr( "Failed to write vector tiles: " ) + writer.errorMessage() );

  return outputs;
}

//
// QgsWriteVectorTilesXyzAlgorithm
//

QString QgsWriteVectorTilesXyzAlgorithm::name() const
{
  return QStringLiteral( "writevectortiles_xyz" );
}

QString QgsWriteVectorTilesXyzAlgorithm::displayName() const
{
  return QObject::tr( "Write Vector Tiles (XYZ)" );
}

QgsProcessingAlgorithm *QgsWriteVectorTilesXyzAlgorithm::createInstance() const
{
  return new QgsWriteVectorTilesXyzAlgorithm();
}

void QgsWriteVectorTilesXyzAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFolderDestination( QStringLiteral( "OUTPUT_DIRECTORY" ), QObject::tr( "Output directory" ) ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "XYZ_TEMPLATE" ), QObject::tr( "File template" ), QStringLiteral( "{z}/{x}/{y}.pbf" ) ) );

  addBaseParameters();
}

void QgsWriteVectorTilesXyzAlgorithm::prepareWriter( QgsVectorTileWriter &writer, const QVariantMap &parameters, QgsProcessingContext &context, QVariantMap &outputs )
{
  const QString outputDir = parameterAsString( parameters, QStringLiteral( "OUTPUT_DIRECTORY" ), context );
  const QString xyzTemplate = parameterAsString( parameters, QStringLiteral( "XYZ_TEMPLATE" ), context );
  QgsDataSourceUri dsUri;
  dsUri.setParam( QStringLiteral( "type" ), QStringLiteral( "xyz" ) );
  dsUri.setParam( QStringLiteral( "url" ), QUrl::fromLocalFile( outputDir + "/" + xyzTemplate ).toString() );
  const QString uri = dsUri.encodedUri();

  writer.setDestinationUri( uri );

  outputs.insert( QStringLiteral( "OUTPUT_DIRECTORY" ), outputDir );
}

//
// QgsWriteVectorTilesMbtilesAlgorithm
//

QString QgsWriteVectorTilesMbtilesAlgorithm::name() const
{
  return QStringLiteral( "writevectortiles_mbtiles" );
}

QString QgsWriteVectorTilesMbtilesAlgorithm::displayName() const
{
  return QObject::tr( "Write Vector Tiles (MBTiles)" );
}

QgsProcessingAlgorithm *QgsWriteVectorTilesMbtilesAlgorithm::createInstance() const
{
  return new QgsWriteVectorTilesMbtilesAlgorithm();
}

void QgsWriteVectorTilesMbtilesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterVectorTileDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Destination MBTiles" ) ) );

  addBaseParameters();

  // optional metadata for MBTiles
  addParameter( new QgsProcessingParameterString( QStringLiteral( "META_NAME" ), QObject::tr( "Metadata: Name" ), QVariant(), false, true ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "META_DESCRIPTION" ), QObject::tr( "Metadata: Description" ), QVariant(), false, true ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "META_ATTRIBUTION" ), QObject::tr( "Metadata: Attribution" ), QVariant(), false, true ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "META_VERSION" ), QObject::tr( "Metadata: Version" ), QVariant(), false, true ) );
  std::unique_ptr< QgsProcessingParameterString > metaTypeParam = std::make_unique< QgsProcessingParameterString >( QStringLiteral( "META_TYPE" ), QObject::tr( "Metadata: Type" ), QVariant(), false, true );
  metaTypeParam->setMetadata( {{
      QStringLiteral( "widget_wrapper" ), QVariantMap(
      {{QStringLiteral( "value_hints" ), QStringList() << QStringLiteral( "overlay" ) << QStringLiteral( "baselayer" ) }}
      )
    }
  } );
  addParameter( metaTypeParam.release() );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "META_CENTER" ), QObject::tr( "Metadata: Center" ), QVariant(), false, true ) );
}

void QgsWriteVectorTilesMbtilesAlgorithm::prepareWriter( QgsVectorTileWriter &writer, const QVariantMap &parameters, QgsProcessingContext &context, QVariantMap &outputs )
{
  const QString outputFile = parameterAsFileOutput( parameters, QStringLiteral( "OUTPUT" ), context );
  QgsDataSourceUri dsUri;
  dsUri.setParam( QStringLiteral( "type" ), QStringLiteral( "mbtiles" ) );
  dsUri.setParam( QStringLiteral( "url" ), outputFile );
  const QString uri = dsUri.encodedUri();

  writer.setDestinationUri( uri );

  const QString metaName = parameterAsString( parameters, QStringLiteral( "META_NAME" ), context );
  const QString metaDescription = parameterAsString( parameters, QStringLiteral( "META_DESCRIPTION" ), context );
  const QString metaAttribution = parameterAsString( parameters, QStringLiteral( "META_ATTRIBUTION" ), context );
  const QString metaVersion = parameterAsString( parameters, QStringLiteral( "META_VERSION" ), context );
  const QString metaType = parameterAsString( parameters, QStringLiteral( "META_TYPE" ), context );
  const QString metaCenter = parameterAsString( parameters, QStringLiteral( "META_CENTER" ), context );

  QVariantMap meta;
  if ( !metaName.isEmpty() )
    meta["name"] = metaName;
  if ( !metaDescription.isEmpty() )
    meta["description"] = metaDescription;
  if ( !metaAttribution.isEmpty() )
    meta["attribution"] = metaAttribution;
  if ( !metaVersion.isEmpty() )
    meta["version"] = metaVersion;
  if ( !metaType.isEmpty() )
    meta["type"] = metaType;
  if ( !metaCenter.isEmpty() )
    meta["center"] = metaCenter;

  writer.setMetadata( meta );

  outputs.insert( QStringLiteral( "OUTPUT" ), outputFile );
}


///@endcond
