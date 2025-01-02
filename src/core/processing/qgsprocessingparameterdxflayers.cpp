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
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  QgsMapLayer *mapLayer = nullptr;
  QgsVectorLayer *vectorLayer = input.value<QgsVectorLayer *>();
  if ( vectorLayer )
  {
    return vectorLayer->isSpatial();
  }

  if ( input.userType() == QMetaType::Type::QString )
  {
    if ( input.toString().isEmpty() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    if ( !context )
      return true;

    mapLayer = QgsProcessingUtils::mapLayerFromString( input.toString(), *context );
    return mapLayer && ( mapLayer->type() == Qgis::LayerType::Vector && mapLayer->isSpatial() );
  }
  else if ( input.userType() == QMetaType::Type::QVariantList )
  {
    if ( input.toList().isEmpty() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    const QVariantList layerList = input.toList();
    for ( const QVariant &variantLayer : layerList )
    {
      vectorLayer = input.value<QgsVectorLayer *>();
      if ( vectorLayer )
      {
        if ( vectorLayer->isSpatial() )
          continue;
        else
          return false;
      }

      if ( variantLayer.userType() == QMetaType::Type::QString )
      {
        if ( !context )
          return true;

        mapLayer = QgsProcessingUtils::mapLayerFromString( variantLayer.toString(), *context );
        if ( !mapLayer || mapLayer->type() != Qgis::LayerType::Vector || !mapLayer->isSpatial() )
          return false;
      }
      else if ( variantLayer.userType() == QMetaType::Type::QVariantMap )
      {
        const QVariantMap layerMap = variantLayer.toMap();

        if ( !layerMap.contains( QStringLiteral( "layer" ) ) &&
             !layerMap.contains( QStringLiteral( "attributeIndex" ) ) &&
             !layerMap.contains( QStringLiteral( "overriddenLayerName" ) ) &&
             !layerMap.contains( QStringLiteral( "buildDataDefinedBlocks" ) ) &&
             !layerMap.contains( QStringLiteral( "dataDefinedBlocksMaximumNumberOfClasses" ) ) )
          return false;

        if ( !context )
          return true;

        mapLayer = QgsProcessingUtils::mapLayerFromString( layerMap.value( QStringLiteral( "layer" ) ).toString(), *context );
        if ( !mapLayer || mapLayer->type() != Qgis::LayerType::Vector || !mapLayer->isSpatial() )
          return false;

        vectorLayer = static_cast<QgsVectorLayer *>( mapLayer );

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
  else if ( input.userType() == QMetaType::Type::QStringList )
  {
    const auto constToStringList = input.toStringList();
    if ( constToStringList.isEmpty() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    if ( !context )
      return true;

    for ( const QString &v : constToStringList )
    {
      mapLayer = QgsProcessingUtils::mapLayerFromString( v, *context );
      if ( !mapLayer )
        return false;

      if ( mapLayer->type() == Qgis::LayerType::Vector && mapLayer->isSpatial() )
        continue;
      else
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

    layerDefParts << QStringLiteral( "'overriddenLayerName': " ) + QgsProcessingUtils::stringToPythonLiteral( layer.overriddenName() );

    layerDefParts << QStringLiteral( "'buildDataDefinedBlocks': " ) + QgsProcessingUtils::variantToPythonLiteral( layer.buildDataDefinedBlocks() );

    layerDefParts << QStringLiteral( "'dataDefinedBlocksMaximumNumberOfClasses': " ) + QgsProcessingUtils::variantToPythonLiteral( layer.dataDefinedBlocksMaximumNumberOfClasses() );

    const QString layerDef = QStringLiteral( "{%1}" ).arg( layerDefParts.join( ',' ) );
    parts << layerDef;
  }
  return parts.join( ',' ).prepend( '[' ).append( ']' );
}

QString QgsProcessingParameterDxfLayers::asPythonString( QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
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

  if ( layersVariant.userType() == QMetaType::Type::QString )
  {
    QgsMapLayer *mapLayer = QgsProcessingUtils::mapLayerFromString( layersVariant.toString(), context );
    layers << QgsDxfExport::DxfLayer( static_cast<QgsVectorLayer *>( mapLayer ) );
  }
  else if ( layersVariant.userType() == QMetaType::Type::QVariantList )
  {
    const QVariantList layersVariantList = layersVariant.toList();
    for ( const QVariant &layerItem : layersVariantList )
    {
      if ( layerItem.userType() == QMetaType::Type::QVariantMap )
      {
        const QVariantMap layerVariantMap = layerItem.toMap();
        layers << variantMapAsLayer( layerVariantMap, context );
      }
      else if ( layerItem.userType() == QMetaType::Type::QString )
      {
        QgsMapLayer *mapLayer = QgsProcessingUtils::mapLayerFromString( layerItem.toString(), context );
        layers << QgsDxfExport::DxfLayer( static_cast<QgsVectorLayer *>( mapLayer ) );
      }
    }
  }
  else if ( layersVariant.userType() == QMetaType::Type::QStringList )
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

  QgsDxfExport::DxfLayer dxfLayer( inputLayer,
                                   layerVariantMap[ QStringLiteral( "attributeIndex" ) ].toInt(),
                                   layerVariantMap[ QStringLiteral( "buildDataDefinedBlocks" ) ].toBool(),
                                   layerVariantMap[ QStringLiteral( "dataDefinedBlocksMaximumNumberOfClasses" ) ].toInt(),
                                   layerVariantMap[ QStringLiteral( "overriddenLayerName" ) ].toString() );
  return dxfLayer;
}

QVariantMap QgsProcessingParameterDxfLayers::layerAsVariantMap( const QgsDxfExport::DxfLayer &layer )
{
  QVariantMap vm;
  if ( !layer.layer() )
    return vm;

  vm[ QStringLiteral( "layer" )] = layer.layer()->id();
  vm[ QStringLiteral( "attributeIndex" ) ] = layer.layerOutputAttributeIndex();
  vm[ QStringLiteral( "overriddenLayerName" ) ] = layer.overriddenName();
  vm[ QStringLiteral( "buildDataDefinedBlocks" ) ] = layer.buildDataDefinedBlocks();
  vm[ QStringLiteral( "dataDefinedBlocksMaximumNumberOfClasses" ) ] = layer.dataDefinedBlocksMaximumNumberOfClasses();
  return vm;
}
