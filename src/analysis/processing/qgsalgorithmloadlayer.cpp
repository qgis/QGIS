/***************************************************************************
                         qgsalgorithmloadlayer.cpp
                         ---------------------
    begin                : November 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgsalgorithmloadlayer.h"

///@cond PRIVATE

QString QgsLoadLayerAlgorithm::name() const
{
  return QStringLiteral( "loadlayer" );
}

QgsProcessingAlgorithm::Flags QgsLoadLayerAlgorithm::flags() const
{
  return FlagHideFromToolbox | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

QString QgsLoadLayerAlgorithm::displayName() const
{
  return QObject::tr( "Load layer into project" );
}

QStringList QgsLoadLayerAlgorithm::tags() const
{
  return QObject::tr( "load,open,layer,raster,vector,project" ).split( ',' );
}

QString QgsLoadLayerAlgorithm::group() const
{
  return QObject::tr( "Modeler tools" );
}

QString QgsLoadLayerAlgorithm::groupId() const
{
  return QStringLiteral( "modelertools" );
}

QString QgsLoadLayerAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm loads a layer to the current project." );
}

QgsLoadLayerAlgorithm *QgsLoadLayerAlgorithm::createInstance() const
{
  return new QgsLoadLayerAlgorithm();
}

void QgsLoadLayerAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMapLayer( QStringLiteral( "INPUT" ), QObject::tr( "Layer" ) ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "NAME" ), QObject::tr( "Loaded layer name" ) ) );
  addOutput( new QgsProcessingOutputMapLayer( QStringLiteral( "OUTPUT" ), QObject::tr( "Layer" ) ) );
}

QVariantMap QgsLoadLayerAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsMapLayer *layer = parameterAsLayer( parameters, QStringLiteral( "INPUT" ), context );
  QString name = parameterAsString( parameters, QStringLiteral( "NAME" ), context );

  if ( !layer )
    throw QgsProcessingException( QObject::tr( "Invalid input layer" ) );

  if ( name.isEmpty() )
    throw QgsProcessingException( QObject::tr( "Invalid (empty) layer name" ) );

  layer->setName( name );
  context.addLayerToLoadOnCompletion( layer->id(), QgsProcessingContext::LayerDetails( name, context.project(), name ) );

  QVariantMap results;
  results.insert( QStringLiteral( "OUTPUT" ), layer->id() );
  return results;
}

///@endcond
