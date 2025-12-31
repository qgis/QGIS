/***************************************************************************
  qgsprocessingparametertininputlayers.cpp
  ---------------------
  Date                 : August 2020
  Copyright            : (C) 2020 by Vincent Cloarec
  Email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingparametertininputlayers.h"

#include "qgsvectorlayer.h"

QgsProcessingParameterTinInputLayers::QgsProcessingParameterTinInputLayers( const QString &name, const QString &description ):
  QgsProcessingParameterDefinition( name, description )
{}

QgsProcessingParameterDefinition *QgsProcessingParameterTinInputLayers::clone() const
{
  return new QgsProcessingParameterTinInputLayers( name(), description() );
}

QString QgsProcessingParameterTinInputLayers::type() const
{
  return typeName();
}

bool QgsProcessingParameterTinInputLayers::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context ) const
{
  if ( input.userType() != QMetaType::Type::QVariantList )
    return false;

  const QVariantList variantLayers = input.toList();

  if ( variantLayers.isEmpty() )
    return false;

  for ( const QVariant &variantLayer : variantLayers )
  {
    if ( variantLayer.userType() != QMetaType::Type::QVariantMap )
      return false;
    const QVariantMap layerMap = variantLayer.toMap();

    if ( !layerMap.contains( u"source"_s ) ||
         !layerMap.contains( u"type"_s ) ||
         !layerMap.contains( u"attributeIndex"_s ) )
      return false;

    if ( !context )
      continue;  // when called without context, we will skip checking whether the layer can be resolved

    QgsMapLayer *mapLayer = QgsProcessingUtils::mapLayerFromString( layerMap.value( u"source"_s ).toString(), *context );
    if ( !mapLayer || mapLayer->type() != Qgis::LayerType::Vector )
      return false;

    QgsVectorLayer *vectorLayer = static_cast<QgsVectorLayer *>( mapLayer );

    if ( layerMap.value( u"attributeIndex"_s ).toInt() >= vectorLayer->fields().count() )
      return false;
  }

  return true;
}

QString QgsProcessingParameterTinInputLayers::valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const
{
  Q_UNUSED( context );
  QStringList parts;
  const QVariantList variantLayers = value.toList();
  for ( const QVariant &variantLayer : variantLayers )
  {
    const QVariantMap layerMap = variantLayer.toMap();
    QStringList layerDefParts;
    layerDefParts << u"'source': "_s + QgsProcessingUtils::variantToPythonLiteral( layerMap.value( u"source"_s ) );
    layerDefParts << u"'type': "_s + QgsProcessingUtils::variantToPythonLiteral( layerMap.value( u"type"_s ) );
    layerDefParts << u"'attributeIndex': "_s + QgsProcessingUtils::variantToPythonLiteral( layerMap.value( u"attributeIndex"_s ) );
    const QString layerDef = u"{%1}"_s.arg( layerDefParts.join( ',' ) );
    parts.append( layerDef );
  }
  return parts.join( ',' ).prepend( '[' ).append( ']' );
}

QString QgsProcessingParameterTinInputLayers::valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok ) const
{
  return valueAsStringPrivate( value, context, ok, ValueAsStringFlag::AllowMapLayerValues );
}

QVariant QgsProcessingParameterTinInputLayers::valueAsJsonObject( const QVariant &value, QgsProcessingContext &context ) const
{
  return valueAsJsonObjectPrivate( value, context, ValueAsStringFlag::AllowMapLayerValues );
}

QString QgsProcessingParameterTinInputLayers::asPythonString( QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterTinInputLayers('%1', %2)"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      return code;
    }
  }
  return QString();
}
