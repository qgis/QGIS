/***************************************************************************
                         qgsalgorithmsetlayerencoding.cpp
                         ------------------------------
    begin                : February 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmsetlayerencoding.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

///@cond PRIVATE

QString QgsSetLayerEncodingAlgorithm::name() const
{
  return QStringLiteral( "setlayerencoding" );
}

QString QgsSetLayerEncodingAlgorithm::displayName() const
{
  return QObject::tr( "Set layer encoding" );
}

QStringList QgsSetLayerEncodingAlgorithm::tags() const
{
  return QObject::tr( "change,alter,attribute,codepage" ).split( ',' );
}

QString QgsSetLayerEncodingAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsSetLayerEncodingAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeneral" );
}

QString QgsSetLayerEncodingAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm sets the encoding used for reading a layer's attributes. No permanent changes "
                      "are made to the layer, rather it affects only how the layer is read during the current session.\n\n"
                      "Changing the encoding is only supported for some vector layer data sources." );
}

QString QgsSetLayerEncodingAlgorithm::shortDescription() const
{
  return QObject::tr( "Sets the encoding used for reading a layer's attributes" );
}

QgsSetLayerEncodingAlgorithm *QgsSetLayerEncodingAlgorithm::createInstance() const
{
  return new QgsSetLayerEncodingAlgorithm();
}

void QgsSetLayerEncodingAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterVectorLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "ENCODING" ), QObject::tr( "Encoding" ) ) );

  addOutput( new QgsProcessingOutputVectorLayer( QStringLiteral( "OUTPUT" ), QObject::tr( "Output layer" ) ) );
}

bool QgsSetLayerEncodingAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsVectorLayer *layer = parameterAsVectorLayer( parameters, QStringLiteral( "INPUT" ), context );

  if ( !layer )
    throw QgsProcessingException( QObject::tr( "Could not load source layer for %1." ).arg( QLatin1String( "INPUT" ) ) );

  const QString encoding = parameterAsString( parameters, QStringLiteral( "ENCODING" ), context );

  mOutputId = layer->id();
  QgsVectorDataProvider *provider = layer->dataProvider();

  if ( provider->capabilities() & Qgis::VectorProviderCapability::SelectEncoding )
  {
    layer->setProviderEncoding( encoding );
  }
  else
  {
    feedback->pushInfo( QObject::tr( "Layer's data provider does not support changing the attribute encoding" ) );
    // we don't return false here -- rather we allow the algorithm to gracefully handle an attempt to be flexible to different layer sources,
    // otherwise we potentially break model input flexibility!
  }
  return true;
}

QVariantMap QgsSetLayerEncodingAlgorithm::processAlgorithm( const QVariantMap &, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), mOutputId );
  return outputs;
}

///@endcond
