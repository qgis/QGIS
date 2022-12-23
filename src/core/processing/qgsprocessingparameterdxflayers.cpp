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

  if ( qobject_cast< QgsVectorLayer * >( qvariant_cast<QObject *>( input ) ) )
  {
    return true;
  }

  if ( input.type() == QVariant::String )
  {
    if ( input.toString().isEmpty() )
      return mFlags & FlagOptional;

    if ( !context )
      return true;

    QgsMapLayer *mapLayer = QgsProcessingUtils::mapLayerFromString( input.toString(), *context );
    return mapLayer && ( mapLayer->type() == QgsMapLayerType::VectorLayer );
  }
  else if ( input.type() == QVariant::List )
  {
    if ( input.toList().isEmpty() )
      return mFlags & FlagOptional;;

    const QVariantList layerList = input.toList();
    for ( const QVariant &variantLayer : layerList )
    {
      if ( qobject_cast< QgsVectorLayer * >( qvariant_cast<QObject *>( variantLayer ) ) )
        continue;

      if ( variantLayer.type() == QVariant::String )
      {
        if ( !context )
          return true;

        QgsMapLayer *mapLayer = QgsProcessingUtils::mapLayerFromString( variantLayer.toString(), *context );
        if ( !mapLayer || mapLayer->type() != QgsMapLayerType::VectorLayer )
          return false;
      }
      else if ( variantLayer.type() == QVariant::Map )
      {
        const QVariantMap layerMap = variantLayer.toMap();

        if ( !layerMap.contains( QStringLiteral( "layer" ) ) && !layerMap.contains( QStringLiteral( "attributeIndex" ) ) )
          return false;

        if ( !context )
          return true;

        QgsMapLayer *mapLayer = QgsProcessingUtils::mapLayerFromString( layerMap.value( QStringLiteral( "layer" ) ).toString(), *context );
        if ( !mapLayer || mapLayer->type() != QgsMapLayerType::VectorLayer )
          return false;

        QgsVectorLayer *vectorLayer = static_cast<QgsVectorLayer *>( mapLayer );

        if ( !vectorLayer )
          return false;

        if ( layerMap.value( QStringLiteral( "attributeIndex" ) ).toInt() >= vectorLayer->fields().count() )
          return false;
      }
      else
      {
        return false;
      }
    }
    return true;
  }
  else if ( input.type() == QVariant::StringList )
  {
    const auto constToStringList = input.toStringList();
    if ( constToStringList.isEmpty() )
      return mFlags & FlagOptional;

    if ( !context )
      return true;

    for ( const QString &v : constToStringList )
    {
      if ( !QgsProcessingUtils::mapLayerFromString( v, *context ) )
        return false;
    }
    return true;
  }

  return false;
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

    const QString layerDef = QStringLiteral( "{%1}" ).arg( layerDefParts.join( ',' ) );
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
      QString code = QStringLiteral( "QgsProcessingParameterDxfLayers('%1', %2)" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      return code;
    }
  }
  return QString();
}

QString QgsProcessingParameterDxfLayers::valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok ) const
{
  return valueAsStringPrivate( value, context, ok, ValueAsStringFlag::AllowMapLayerValues );
}

QVariant QgsProcessingParameterDxfLayers::valueAsJsonObject( const QVariant &value, QgsProcessingContext &context ) const
{
  return valueAsJsonObjectPrivate( value, context, ValueAsStringFlag::AllowMapLayerValues );
}

QList<QgsDxfExport::DxfLayer> QgsProcessingParameterDxfLayers::parameterAsLayers( const QVariant &layersVariant, QgsProcessingContext &context )
{
  QList<QgsDxfExport::DxfLayer> layers;

  if ( QgsVectorLayer *layer = qobject_cast< QgsVectorLayer * >( qvariant_cast<QObject *>( layersVariant ) ) )
  {
    layers << QgsDxfExport::DxfLayer( layer );
  }

  if ( layersVariant.type() == QVariant::String )
  {
    QgsMapLayer *mapLayer = QgsProcessingUtils::mapLayerFromString( layersVariant.toString(), context );
    layers << QgsDxfExport::DxfLayer( static_cast<QgsVectorLayer *>( mapLayer ) );
  }
  else if ( layersVariant.type() == QVariant::List )
  {
    const QVariantList layersVariantList = layersVariant.toList();
    for ( const QVariant &layerItem : layersVariantList )
    {
      if ( layerItem.type() == QVariant::Map )
      {
        const QVariantMap layerVariantMap = layerItem.toMap();
        layers << variantMapAsLayer( layerVariantMap, context );
      }
      else if ( layerItem.type() == QVariant::String )
      {
        QgsMapLayer *mapLayer = QgsProcessingUtils::mapLayerFromString( layerItem.toString(), context );
        layers << QgsDxfExport::DxfLayer( static_cast<QgsVectorLayer *>( mapLayer ) );
      }
    }
  }
  else if ( layersVariant.type() == QVariant::StringList )
  {
    const auto layersStringList = layersVariant.toStringList();
    for ( const QString &layerItem : layersStringList )
    {
      QgsMapLayer *mapLayer = QgsProcessingUtils::mapLayerFromString( layerItem, context );
      layers << QgsDxfExport::DxfLayer( static_cast<QgsVectorLayer *>( mapLayer ) );
    }
  }

  return layers;
}

QgsDxfExport::DxfLayer QgsProcessingParameterDxfLayers::variantMapAsLayer( const QVariantMap &layerVariantMap, QgsProcessingContext &context )
{
  const QVariant layerVariant = layerVariantMap[ QStringLiteral( "layer" ) ];

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
