/***************************************************************************
  qgsprocessingparameteralignrasterlayers.cpp
  ---------------------
  Date                 : July 2023
  Copyright            : (C) 2023 by Alexander Bruy
  Email                : alexander dot bruy at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingparameteralignrasterlayers.h"

#include "qgis.h"
#include "qgsrasterlayer.h"

QgsProcessingParameterAlignRasterLayers::QgsProcessingParameterAlignRasterLayers( const QString &name, const QString &description )
  : QgsProcessingParameterDefinition( name, description, QVariant(), false )
{
}

QgsProcessingParameterDefinition *QgsProcessingParameterAlignRasterLayers::clone() const
{
  return new QgsProcessingParameterAlignRasterLayers( *this );
}

QString QgsProcessingParameterAlignRasterLayers::type() const
{
  return typeName();
}

bool QgsProcessingParameterAlignRasterLayers::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context ) const
{
  if ( !input.isValid() )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  if ( qobject_cast< QgsRasterLayer * >( qvariant_cast<QObject *>( input ) ) )
  {
    return true;
  }

  if ( input.userType() == QMetaType::Type::QString )
  {
    if ( input.toString().isEmpty() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    if ( !context )
      return true;

    QgsMapLayer *mapLayer = QgsProcessingUtils::mapLayerFromString( input.toString(), *context );
    return mapLayer && ( mapLayer->type() == Qgis::LayerType::Raster );
  }
  else if ( input.userType() == QMetaType::Type::QVariantList )
  {
    if ( input.toList().isEmpty() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;;

    const QVariantList layerList = input.toList();
    for ( const QVariant &variantLayer : layerList )
    {
      if ( qobject_cast< QgsRasterLayer * >( qvariant_cast<QObject *>( variantLayer ) ) )
        continue;

      if ( variantLayer.userType() == QMetaType::Type::QString )
      {
        if ( !context )
          return true;

        QgsMapLayer *mapLayer = QgsProcessingUtils::mapLayerFromString( variantLayer.toString(), *context );
        if ( !mapLayer || mapLayer->type() != Qgis::LayerType::Raster )
          return false;
      }
      else if ( variantLayer.userType() == QMetaType::Type::QVariantMap )
      {
        const QVariantMap layerMap = variantLayer.toMap();

        if ( !layerMap.contains( u"inputFile"_s ) && !layerMap.contains( u"outputFile"_s ) )
          return false;

        if ( !context )
          return true;

        QgsRasterLayer *rasterLayer = qobject_cast< QgsRasterLayer * >(
                                        QgsProcessingUtils::mapLayerFromString( layerMap.value( u"inputFile"_s ).toString(), *context )
                                      );
        if ( !rasterLayer )
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
      if ( !QgsProcessingUtils::mapLayerFromString( v, *context ) )
        return false;
    }
    return true;
  }

  return false;
}

QString QgsProcessingParameterAlignRasterLayers::valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const
{
  QStringList parts;
  const QList<QgsAlignRasterData::RasterItem> items = parameterAsItems( value, context );
  for ( const QgsAlignRasterData::RasterItem &item : items )
  {
    QStringList layerDefParts;
    layerDefParts << u"'inputFile': "_s + QgsProcessingUtils::stringToPythonLiteral( QgsProcessingUtils::normalizeLayerSource( item.inputFilename ) );
    layerDefParts << u"'outputFile': "_s + QgsProcessingUtils::stringToPythonLiteral( QgsProcessingUtils::normalizeLayerSource( item.outputFilename ) );
    layerDefParts << u"'resampleMethod': "_s + QgsProcessingUtils::variantToPythonLiteral( static_cast<int>( item.resampleMethod ) );
    layerDefParts << u"'rescale': "_s + QgsProcessingUtils::variantToPythonLiteral( item.rescaleValues );

    const QString layerDef = u"{%1}"_s.arg( layerDefParts.join( ',' ) );
    parts << layerDef;
  }
  return parts.join( ',' ).prepend( '[' ).append( ']' );
}

QString QgsProcessingParameterAlignRasterLayers::asPythonString( QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterAlignRasterLayers('%1', %2)"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      return code;
    }
  }
  return QString();
}

QString QgsProcessingParameterAlignRasterLayers::valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok ) const
{
  return valueAsStringPrivate( value, context, ok, ValueAsStringFlag::AllowMapLayerValues );
}

QVariant QgsProcessingParameterAlignRasterLayers::valueAsJsonObject( const QVariant &value, QgsProcessingContext &context ) const
{
  return valueAsJsonObjectPrivate( value, context, ValueAsStringFlag::AllowMapLayerValues );
}

QList<QgsAlignRasterData::RasterItem> QgsProcessingParameterAlignRasterLayers::parameterAsItems( const QVariant &layersVariant, QgsProcessingContext &context )
{
  QList<QgsAlignRasterData::RasterItem> items;

  if ( qobject_cast< QgsRasterLayer * >( qvariant_cast<QObject *>( layersVariant ) ) )
  {
    QVariantMap vm;
    vm["inputFile"] = layersVariant;
    items << variantMapAsItem( vm, context );
  }

  if ( layersVariant.userType() == QMetaType::Type::QString )
  {
    QVariantMap vm;
    vm["inputFile"] = layersVariant;
    items << variantMapAsItem( vm, context );
  }
  else if ( layersVariant.userType() == QMetaType::Type::QVariantList )
  {
    const QVariantList layersVariantList = layersVariant.toList();
    for ( const QVariant &layerItem : layersVariantList )
    {
      if ( layerItem.userType() == QMetaType::Type::QVariantMap )
      {
        const QVariantMap layerVariantMap = layerItem.toMap();
        items << variantMapAsItem( layerVariantMap, context );
      }
      else if ( layerItem.userType() == QMetaType::Type::QString )
      {
        QVariantMap vm;
        vm["inputFile"] = layerItem;
        items << variantMapAsItem( vm, context );
      }
    }
  }
  else if ( layersVariant.userType() == QMetaType::Type::QStringList )
  {
    const auto layersStringList = layersVariant.toStringList();
    for ( const QString &layerItem : layersStringList )
    {
      QVariantMap vm;
      vm["inputFile"] = layerItem;
      items << variantMapAsItem( vm, context );
    }
  }
  else if ( layersVariant.userType() == QMetaType::Type::QVariantMap )
  {
    const QVariantMap layerVariantMap = layersVariant.toMap();
    items << variantMapAsItem( layerVariantMap, context );
  }

  return items;
}

QgsAlignRasterData::RasterItem QgsProcessingParameterAlignRasterLayers::variantMapAsItem( const QVariantMap &layerVariantMap, QgsProcessingContext &context )
{
  const QVariant layerVariant = layerVariantMap[ u"inputFile"_s ];

  QgsRasterLayer *inputLayer = nullptr;
  if ( ( inputLayer = qobject_cast< QgsRasterLayer * >( qvariant_cast<QObject *>( layerVariant ) ) ) )
  {
    // good
  }
  else if ( ( inputLayer = qobject_cast< QgsRasterLayer * >( QgsProcessingUtils::mapLayerFromString( layerVariant.toString(), context ) ) ) )
  {
    // good
  }
  else
  {
    QgsAlignRasterData::RasterItem item( "", "" );
    return item;
  }

  QgsAlignRasterData::RasterItem item( inputLayer->source(), layerVariantMap[ u"outputFile"_s ].toString() );
  item.resampleMethod = static_cast<Qgis::GdalResampleAlgorithm>( layerVariantMap.value( u"resampleMethod"_s, 0 ).toInt() );
  item.rescaleValues = layerVariantMap.value( u"rescale"_s, false ).toBool();
  return item;
}

QVariantMap QgsProcessingParameterAlignRasterLayers::itemAsVariantMap( const QgsAlignRasterData::RasterItem &item )
{
  QVariantMap vm;
  vm[ u"inputFile"_s] = item.inputFilename;
  vm[ u"outputFile"_s ] = item.outputFilename;
  vm[ u"resampleMethod"_s ] = static_cast<int>( item.resampleMethod );
  vm[ u"rescale"_s ] = item.rescaleValues;
  return vm;
}
