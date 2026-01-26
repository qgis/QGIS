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
  return u"vectortiles"_s;
}

QString QgsWriteVectorTilesBaseAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm exports one or more vector layers to vector tiles - a data format optimized for fast map rendering and small data size." );
}

QString QgsWriteVectorTilesBaseAlgorithm::shortDescription() const
{
  return QObject::tr( "Exports one or more vector layers to vector tiles." );
}

void QgsWriteVectorTilesBaseAlgorithm::addBaseParameters()
{
  addParameter( new QgsProcessingParameterVectorTileWriterLayers( u"LAYERS"_s, QObject::tr( "Input layers" ) ) );

  addParameter( new QgsProcessingParameterNumber( u"MIN_ZOOM"_s, QObject::tr( "Minimum zoom level" ), Qgis::ProcessingNumberParameterType::Integer, 0, false, 0, 24 ) );
  addParameter( new QgsProcessingParameterNumber( u"MAX_ZOOM"_s, QObject::tr( "Maximum zoom level" ), Qgis::ProcessingNumberParameterType::Integer, 3, false, 0, 24 ) );

  // optional extent
  addParameter( new QgsProcessingParameterExtent( u"EXTENT"_s, QObject::tr( "Extent" ), QVariant(), true ) );
}

QVariantMap QgsWriteVectorTilesBaseAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const int minZoom = parameterAsInt( parameters, u"MIN_ZOOM"_s, context );
  const int maxZoom = parameterAsInt( parameters, u"MAX_ZOOM"_s, context );

  const QVariant layersVariant = parameters.value( parameterDefinition( u"LAYERS"_s )->name() );
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

  if ( parameters.contains( u"EXTENT"_s ) )
  {
    const QgsRectangle extent = parameterAsExtent( parameters, u"EXTENT"_s, context, QgsCoordinateReferenceSystem( "EPSG:3857" ) );
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
  return u"writevectortiles_xyz"_s;
}

QString QgsWriteVectorTilesXyzAlgorithm::displayName() const
{
  return QObject::tr( "Write Vector Tiles (XYZ)" );
}

QStringList QgsWriteVectorTilesXyzAlgorithm::tags() const
{
  return QObject::tr( "xyz,vector,tiles" ).split( ',' );
}

QgsProcessingAlgorithm *QgsWriteVectorTilesXyzAlgorithm::createInstance() const
{
  return new QgsWriteVectorTilesXyzAlgorithm();
}

void QgsWriteVectorTilesXyzAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFolderDestination( u"OUTPUT_DIRECTORY"_s, QObject::tr( "Output directory" ) ) );
  addParameter( new QgsProcessingParameterString( u"XYZ_TEMPLATE"_s, QObject::tr( "File template" ), u"{z}/{x}/{y}.pbf"_s ) );

  addBaseParameters();
}

void QgsWriteVectorTilesXyzAlgorithm::prepareWriter( QgsVectorTileWriter &writer, const QVariantMap &parameters, QgsProcessingContext &context, QVariantMap &outputs )
{
  const QString outputDir = parameterAsString( parameters, u"OUTPUT_DIRECTORY"_s, context );
  const QString xyzTemplate = parameterAsString( parameters, u"XYZ_TEMPLATE"_s, context );
  QgsDataSourceUri dsUri;
  dsUri.setParam( u"type"_s, u"xyz"_s );
  dsUri.setParam( u"url"_s, QUrl::fromLocalFile( outputDir + "/" + xyzTemplate ).toString() );
  const QString uri = dsUri.encodedUri();

  writer.setDestinationUri( uri );

  outputs.insert( u"OUTPUT_DIRECTORY"_s, outputDir );
}

//
// QgsWriteVectorTilesMbtilesAlgorithm
//

QString QgsWriteVectorTilesMbtilesAlgorithm::name() const
{
  return u"writevectortiles_mbtiles"_s;
}

QString QgsWriteVectorTilesMbtilesAlgorithm::displayName() const
{
  return QObject::tr( "Write Vector Tiles (MBTiles)" );
}

QStringList QgsWriteVectorTilesMbtilesAlgorithm::tags() const
{
  return QObject::tr( "mbtiles,vector" ).split( ',' );
}

QgsProcessingAlgorithm *QgsWriteVectorTilesMbtilesAlgorithm::createInstance() const
{
  return new QgsWriteVectorTilesMbtilesAlgorithm();
}

void QgsWriteVectorTilesMbtilesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterVectorTileDestination( u"OUTPUT"_s, QObject::tr( "Destination MBTiles" ) ) );

  addBaseParameters();

  // optional metadata for MBTiles
  addParameter( new QgsProcessingParameterString( u"META_NAME"_s, QObject::tr( "Metadata: Name" ), QVariant(), false, true ) );
  addParameter( new QgsProcessingParameterString( u"META_DESCRIPTION"_s, QObject::tr( "Metadata: Description" ), QVariant(), false, true ) );
  addParameter( new QgsProcessingParameterString( u"META_ATTRIBUTION"_s, QObject::tr( "Metadata: Attribution" ), QVariant(), false, true ) );
  addParameter( new QgsProcessingParameterString( u"META_VERSION"_s, QObject::tr( "Metadata: Version" ), QVariant(), false, true ) );
  auto metaTypeParam = std::make_unique<QgsProcessingParameterString>( u"META_TYPE"_s, QObject::tr( "Metadata: Type" ), QVariant(), false, true );
  metaTypeParam->setMetadata( { { u"widget_wrapper"_s, QVariantMap( { { u"value_hints"_s, QStringList() << u"overlay"_s << u"baselayer"_s } } ) }
  } );
  addParameter( metaTypeParam.release() );
  addParameter( new QgsProcessingParameterString( u"META_CENTER"_s, QObject::tr( "Metadata: Center" ), QVariant(), false, true ) );
}

void QgsWriteVectorTilesMbtilesAlgorithm::prepareWriter( QgsVectorTileWriter &writer, const QVariantMap &parameters, QgsProcessingContext &context, QVariantMap &outputs )
{
  const QString outputFile = parameterAsFileOutput( parameters, u"OUTPUT"_s, context );
  QgsDataSourceUri dsUri;
  dsUri.setParam( u"type"_s, u"mbtiles"_s );
  dsUri.setParam( u"url"_s, outputFile );
  const QString uri = dsUri.encodedUri();

  writer.setDestinationUri( uri );

  const QString metaName = parameterAsString( parameters, u"META_NAME"_s, context );
  const QString metaDescription = parameterAsString( parameters, u"META_DESCRIPTION"_s, context );
  const QString metaAttribution = parameterAsString( parameters, u"META_ATTRIBUTION"_s, context );
  const QString metaVersion = parameterAsString( parameters, u"META_VERSION"_s, context );
  const QString metaType = parameterAsString( parameters, u"META_TYPE"_s, context );
  const QString metaCenter = parameterAsString( parameters, u"META_CENTER"_s, context );

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

  outputs.insert( u"OUTPUT"_s, outputFile );
}


///@endcond
