/***************************************************************************
  qgsprocessingparametervectortilewriterlayers.cpp
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

#include "qgsprocessingparametervectortilewriterlayers.h"

#include "qgsvectorlayer.h"


QgsProcessingParameterVectorTileWriterLayers::QgsProcessingParameterVectorTileWriterLayers( const QString &name, const QString &description )
  : QgsProcessingParameterDefinition( name, description, QVariant(), false )
{
}

QgsProcessingParameterDefinition *QgsProcessingParameterVectorTileWriterLayers::clone() const
{
  return new QgsProcessingParameterVectorTileWriterLayers( *this );
}

QString QgsProcessingParameterVectorTileWriterLayers::type() const
{
  return typeName();
}

bool QgsProcessingParameterVectorTileWriterLayers::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context ) const
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

    const QVariant inputItemLayer = inputItemMap["layer"];

    if ( qobject_cast< QgsVectorLayer * >( qvariant_cast<QObject *>( inputItemLayer ) ) )
      continue;

    if ( !context )
      continue;  // when called without context, we will skip checking whether the layer can be resolved

    if ( !QgsProcessingUtils::mapLayerFromString( inputItemLayer.toString(), *context ) )
      return false;
  }

  return true;
}

QString QgsProcessingParameterVectorTileWriterLayers::valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const
{
  QStringList parts;
  const QList<QgsVectorTileWriter::Layer> layers = parameterAsLayers( value, context );
  for ( const QgsVectorTileWriter::Layer &layer : layers )
  {
    QStringList layerDefParts;
    layerDefParts << QStringLiteral( "'layer': " ) + QgsProcessingUtils::stringToPythonLiteral( QgsProcessingUtils::normalizeLayerSource( layer.layer()->source() ) );
    if ( !layer.filterExpression().isEmpty() )
      layerDefParts << QStringLiteral( "'filterExpression': " ) + QgsProcessingUtils::variantToPythonLiteral( layer.filterExpression() );
    if ( !layer.layerName().isEmpty() )
      layerDefParts << QStringLiteral( "'layerName': " ) + QgsProcessingUtils::variantToPythonLiteral( layer.layerName() );
    if ( layer.minZoom() >= 0 )
      layerDefParts << QStringLiteral( "'minZoom': " ) + QgsProcessingUtils::variantToPythonLiteral( layer.minZoom() );
    if ( layer.maxZoom() >= 0 )
      layerDefParts << QStringLiteral( "'maxZoom': " ) + QgsProcessingUtils::variantToPythonLiteral( layer.maxZoom() );

    const QString layerDef = QStringLiteral( "{ %1 }" ).arg( layerDefParts.join( ',' ) );
    parts << layerDef;
  }
  return parts.join( ',' ).prepend( '[' ).append( ']' );
}

QString QgsProcessingParameterVectorTileWriterLayers::asPythonString( QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterVectorTileWriterLayers('%1', %2)" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      return code;
    }
  }
  return QString();
}

QList<QgsVectorTileWriter::Layer> QgsProcessingParameterVectorTileWriterLayers::parameterAsLayers( const QVariant &layersVariant, QgsProcessingContext &context )
{
  QList<QgsVectorTileWriter::Layer> layers;
  const QVariantList layersVariantList = layersVariant.toList();
  for ( const QVariant &layerItem : layersVariantList )
  {
    const QVariantMap layerVariantMap = layerItem.toMap();
    layers << variantMapAsLayer( layerVariantMap, context );
  }
  return layers;
}

QgsVectorTileWriter::Layer QgsProcessingParameterVectorTileWriterLayers::variantMapAsLayer( const QVariantMap &layerVariantMap, QgsProcessingContext &context )
{
  const QVariant layerVariant = layerVariantMap["layer"];

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

  return writerLayer;
}

QVariantMap QgsProcessingParameterVectorTileWriterLayers::layerAsVariantMap( const QgsVectorTileWriter::Layer &layer )
{
  QVariantMap vm;
  if ( !layer.layer() )
    return vm;

  vm["layer"] = layer.layer()->id();
  vm["minZoom"] = layer.minZoom();
  vm["maxZoom"] = layer.maxZoom();
  vm["layerName"] = layer.layerName();
  vm["filterExpression"] = layer.filterExpression();
  return vm;
}
