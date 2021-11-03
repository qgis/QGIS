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
  if ( input.type() != QVariant::List )
    return false;

  const QVariantList variantLayers = input.toList();

  if ( variantLayers.isEmpty() )
    return false;

  for ( const QVariant &variantLayer : variantLayers )
  {
    if ( variantLayer.type() != QVariant::Map )
      return false;
    const QVariantMap layerMap = variantLayer.toMap();

    if ( !layerMap.contains( QStringLiteral( "source" ) ) ||
         !layerMap.contains( QStringLiteral( "type" ) ) ||
         !layerMap.contains( QStringLiteral( "attributeIndex" ) ) )
      return false;

    if ( !context )
      continue;  // when called without context, we will skip checking whether the layer can be resolved

    QgsMapLayer *mapLayer = QgsProcessingUtils::mapLayerFromString( layerMap.value( QStringLiteral( "source" ) ).toString(), *context );
    if ( !mapLayer || mapLayer->type() != QgsMapLayerType::VectorLayer )
      return false;

    QgsVectorLayer *vectorLayer = static_cast<QgsVectorLayer *>( mapLayer );

    if ( layerMap.value( QStringLiteral( "attributeIndex" ) ).toInt() >= vectorLayer->fields().count() )
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
    layerDefParts << QStringLiteral( "'source': " ) + QgsProcessingUtils::variantToPythonLiteral( layerMap.value( QStringLiteral( "source" ) ) );
    layerDefParts << QStringLiteral( "'type': " ) + QgsProcessingUtils::variantToPythonLiteral( layerMap.value( QStringLiteral( "type" ) ) );
    layerDefParts << QStringLiteral( "'attributeIndex': " ) + QgsProcessingUtils::variantToPythonLiteral( layerMap.value( QStringLiteral( "attributeIndex" ) ) );
    const QString layerDef = QStringLiteral( "{%1}" ).arg( layerDefParts.join( ',' ) );
    parts.append( layerDef );
  }
  return parts.join( ',' ).prepend( '[' ).append( ']' );
}

QString QgsProcessingParameterTinInputLayers::asPythonString( QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterTinInputLayers('%1', %2)" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      return code;
    }
  }
  return QString();
}
