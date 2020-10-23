/***************************************************************************
  qgsprocessingparameterdxflayers.cpp
  ---------------------
  Date                 : September 2020
  Copyright            : (C) 2020 by Alexander Bruy
  Email                : alexander dot bruy at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingparameterdxflayers.h"
#include "qgsvectorlayer.h"


QgsProcessingParameterDxfLayers::QgsProcessingParameterDxfLayers( const QString &name, const QString &description )
  : QgsProcessingParameterDefinition( name, description, QVariant(), false )
{
}

QgsProcessingParameterDefinition *QgsProcessingParameterDxfLayers::clone() const
{
  return new QgsProcessingParameterDxfLayers( *this );
}

QString QgsProcessingParameterDxfLayers::type() const
{
  return typeName();
}

bool QgsProcessingParameterDxfLayers::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context ) const
{
  if ( !input.isValid() )
    return mFlags & FlagOptional;

  if ( input.type() != QVariant::List )
    return false;

  const QVariantList layerList = input.toList();
  if ( layerList.isEmpty() )
    return false;

  for ( const QVariant &variantLayer : layerList )
  {
    if ( variantLayer.type() != QVariant::Map )
      return false;

    QVariantMap layerMap = variantLayer.toMap();

    if ( !layerMap.contains( QStringLiteral( "layer" ) ) || !layerMap.contains( QStringLiteral( "attributeIndex" ) ) )
      return false;

    if ( !context )
      continue;  // when called without context, we will skip checking whether the layer can be resolved

    QgsMapLayer *mapLayer = QgsProcessingUtils::mapLayerFromString( layerMap.value( QStringLiteral( "layer" ) ).toString(), *context );
    if ( !mapLayer || mapLayer->type() != QgsMapLayerType::VectorLayer )
      return false;

    QgsVectorLayer *vectorLayer = static_cast<QgsVectorLayer *>( mapLayer );

    if ( !vectorLayer )
      return false;

    if ( layerMap.value( QStringLiteral( "attributeIndex" ) ).toInt() >= vectorLayer->fields().count() )
      return false;
  }

  return true;
}

QString QgsProcessingParameterDxfLayers::valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const
{
  QStringList parts;
  const QList<QgsDxfExport::DxfLayer> layers = parameterAsLayers( value, context );
  for ( const QgsDxfExport::DxfLayer &layer : layers )
  {
    QStringList layerDefParts;
    layerDefParts << QStringLiteral( "'layer': " ) + QgsProcessingUtils::stringToPythonLiteral( QgsProcessingUtils::normalizeLayerSource( layer.layer()->source() ) );
    if ( layer.layerOutputAttributeIndex() >= -1 )
      layerDefParts << QStringLiteral( "'attributeIndex': " ) + QgsProcessingUtils::variantToPythonLiteral( layer.layerOutputAttributeIndex() );

    QString layerDef = QStringLiteral( "{%1}" ).arg( layerDefParts.join( ',' ) );
    parts << layerDef;
  }
  return parts.join( ',' ).prepend( '[' ).append( ']' );
}

QString QgsProcessingParameterDxfLayers::asPythonString( QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterDxfLayers('%1', '%2')" ).arg( name(), description() );
      return code;
    }
  }
  return QString();
}

QList<QgsDxfExport::DxfLayer> QgsProcessingParameterDxfLayers::parameterAsLayers( const QVariant &layersVariant, QgsProcessingContext &context )
{
  QList<QgsDxfExport::DxfLayer> layers;
  const QVariantList layersVariantList = layersVariant.toList();
  for ( const QVariant &layerItem : layersVariantList )
  {
    QVariantMap layerVariantMap = layerItem.toMap();
    layers << variantMapAsLayer( layerVariantMap, context );
  }
  return layers;
}

QgsDxfExport::DxfLayer QgsProcessingParameterDxfLayers::variantMapAsLayer( const QVariantMap &layerVariantMap, QgsProcessingContext &context )
{
  QVariant layerVariant = layerVariantMap[ QStringLiteral( "layer" ) ];

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

  QgsDxfExport::DxfLayer dxfLayer( inputLayer, layerVariantMap[ QStringLiteral( "attributeIndex" ) ].toInt() );
  return dxfLayer;
}

QVariantMap QgsProcessingParameterDxfLayers::layerAsVariantMap( const QgsDxfExport::DxfLayer &layer )
{
  QVariantMap vm;
  if ( !layer.layer() )
    return vm;

  vm[ QStringLiteral( "layer" )] = layer.layer()->id();
  vm[ QStringLiteral( "attributeIndex" ) ] = layer.layerOutputAttributeIndex();
  return vm;
}
