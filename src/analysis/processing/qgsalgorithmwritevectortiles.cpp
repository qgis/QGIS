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

#include "qgsvectorlayer.h"
#include "qgsvectortilewriter.h"

///@cond PRIVATE


class QgsProcessingParameterVectorTileWriterLayers : public QgsProcessingParameterDefinition
{
public:
  QgsProcessingParameterVectorTileWriterLayers( const QString &name, const QString &description = QString() )
    : QgsProcessingParameterDefinition( name, description, QVariant(), false ) {}

  static QString typeName() { return QStringLiteral( "vectortilewriterlayers" ); }
  QgsProcessingParameterDefinition *clone() const override
  {
    return new QgsProcessingParameterVectorTileWriterLayers( *this );
  }
  QString type() const override { return typeName(); }

  bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context ) const override
  {
    if ( !input.isValid() )
      return mFlags & FlagOptional;

    if ( input.type() != QVariant::List )
      return false;

    const QVariantList inputList = input.toList();
    for ( const QVariant &inputItem : inputList )
    {
      if ( inputItem.type() != QVariant::Map )
        return false;
      QVariantMap inputItemMap = inputItem.toMap();

      // "layer" is required - pointing to a vector layer
      if ( !inputItemMap.contains( "layer" ) )
        return false;

      QVariant inputItemLayer = inputItemMap["layer"];

      if ( qobject_cast< QgsVectorLayer * >( qvariant_cast<QObject *>( inputItemLayer ) ) )
        continue;

      if ( !QgsProcessingUtils::mapLayerFromString( inputItemLayer.toString(), *context ) )
        return false;
    }

    return true;
  }

  // TODO: anything else?
  // - valueAsPythonString()
  // - asScriptCode()
  // - asPythonString()

  static QList<QgsVectorTileWriter::Layer> parameterAsLayers( const QVariant &layersVariant, QgsProcessingContext &context )
  {
    QList<QgsVectorTileWriter::Layer> layers;
    const QVariantList layersVariantList = layersVariant.toList();
    for ( const QVariant &layerItem : layersVariantList )
    {
      QVariantMap layerVariantMap = layerItem.toMap();
      QVariant layerVariant = layerVariantMap["layer"];

      QgsVectorLayer *inputLayer = nullptr;
      if ( ( inputLayer = qobject_cast< QgsVectorLayer * >( qvariant_cast<QObject *>( layerVariant ) ) ) )
      {
        // good
      }
      else if ( ( inputLayer = qobject_cast< QgsVectorLayer * >( QgsProcessingUtils::mapLayerFromString( layerVariant.toString(), context ) ) ) )
      {
        // good
      }
      else
      {
        // bad
        throw QgsProcessingException( "unknown input layer" );
      }

      QgsVectorTileWriter::Layer writerLayer( inputLayer );
      if ( layerVariantMap.contains( "filterExpression" ) )
        writerLayer.setFilterExpression( layerVariantMap["filterExpression"].toString() );
      if ( layerVariantMap.contains( "minZoom" ) )
        writerLayer.setMinZoom( layerVariantMap["minZoom"].toInt() );
      if ( layerVariantMap.contains( "maxZoom" ) )
        writerLayer.setMaxZoom( layerVariantMap["maxZoom"].toInt() );
      if ( layerVariantMap.contains( "layerName" ) )
        writerLayer.setLayerName( layerVariantMap["layerName"].toString() );
      layers << writerLayer;
    }
    return layers;
  }

};

QString QgsWriteVectorTilesAlgorithm::name() const
{
  return QStringLiteral( "writevectortiles" );
}

QString QgsWriteVectorTilesAlgorithm::displayName() const
{
  return QObject::tr( "Write Vector Tiles" );
}

QString QgsWriteVectorTilesAlgorithm::group() const
{
  return QObject::tr( "Vector tiles" );
}

QString QgsWriteVectorTilesAlgorithm::groupId() const
{
  return QStringLiteral( "vectortiles" );
}

QString QgsWriteVectorTilesAlgorithm::shortHelpString() const
{
  return QObject::tr( "blah blah" );  // TODO
}

QgsProcessingAlgorithm *QgsWriteVectorTilesAlgorithm::createInstance() const
{
  return new QgsWriteVectorTilesAlgorithm();
}

void QgsWriteVectorTilesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterVectorTileWriterLayers( "LAYERS", QObject::tr("Input layers") ) );

  addParameter( new QgsProcessingParameterString( "XYZ_TEMPLATE", QObject::tr("File template"), "/home/qgis/{z}/{x}/{y}.pbf" ) );
  // TODO maybe use this:
  // addParameter( new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Destination GeoPackage" ), QObject::tr( "GeoPackage files (*.gpkg)" ) ) );

  addParameter( new QgsProcessingParameterNumber( "MIN_ZOOM", QObject::tr("Minimum zoom level"), QgsProcessingParameterNumber::Integer, 0, false, 0, 24 ) );
  addParameter( new QgsProcessingParameterNumber( "MAX_ZOOM", QObject::tr("Maximum zoom level"), QgsProcessingParameterNumber::Integer, 3, false, 0, 24 ) );

  // optional extent
  addParameter( new QgsProcessingParameterExtent( "EXTENT", QObject::tr("Extent"), QVariant(), true ) );

  // TODO: optional metadata (only for MBTiles? and with concrete values rather than single QVariantMap?)
}

QVariantMap QgsWriteVectorTilesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  int minZoom = parameterAsInt( parameters, QStringLiteral( "MIN_ZOOM" ), context );
  int maxZoom = parameterAsInt( parameters, QStringLiteral( "MAX_ZOOM" ), context );

  // prepare output URI
  QString xyzTemplate = parameterAsString( parameters, QStringLiteral( "XYZ_TEMPLATE" ), context );
  QgsDataSourceUri dsUri;
  dsUri.setParam( "type", "xyz" );
  dsUri.setParam( "url", QUrl::fromLocalFile( xyzTemplate ).toString() );
  QString uri = dsUri.encodedUri();

  QVariant layersVariant = parameters.value( parameterDefinition( QStringLiteral( "LAYERS" ) )->name() );
  QList<QgsVectorTileWriter::Layer> layers = QgsProcessingParameterVectorTileWriterLayers::parameterAsLayers( layersVariant, context );

  QgsVectorTileWriter writer;
  writer.setDestinationUri( uri );
  writer.setMinZoom( minZoom );
  writer.setMaxZoom( maxZoom );
  writer.setLayers( layers );
  writer.setTransformContext( context.transformContext() );

  if ( parameters.contains( "EXTENT" ) )
  {
    QgsRectangle extent = parameterAsExtent( parameters, "EXTENT", context, QgsCoordinateReferenceSystem( "EPSG:3857" ) );
    writer.setExtent( extent );
  }

  bool res = writer.writeTiles( feedback );

  QVariantMap outputs;
  outputs["RESULT"] = res;
  return outputs;
}

///@endcond
