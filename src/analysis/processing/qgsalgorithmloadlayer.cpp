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
  return u"loadlayer"_s;
}

Qgis::ProcessingAlgorithmFlags QgsLoadLayerAlgorithm::flags() const
{
  return Qgis::ProcessingAlgorithmFlag::HideFromToolbox | Qgis::ProcessingAlgorithmFlag::NotAvailableInStandaloneTool;
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
  return u"modelertools"_s;
}

QString QgsLoadLayerAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm loads a layer to the current project." );
}

QString QgsLoadLayerAlgorithm::shortDescription() const
{
  return QObject::tr( "Loads a layer to the current project." );
}

QgsLoadLayerAlgorithm *QgsLoadLayerAlgorithm::createInstance() const
{
  return new QgsLoadLayerAlgorithm();
}

void QgsLoadLayerAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMapLayer( u"INPUT"_s, QObject::tr( "Layer" ) ) );
  addParameter( new QgsProcessingParameterString( u"NAME"_s, QObject::tr( "Loaded layer name" ) ) );
  addOutput( new QgsProcessingOutputMapLayer( u"OUTPUT"_s, QObject::tr( "Layer" ) ) );
}

QVariantMap QgsLoadLayerAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsMapLayer *layer = parameterAsLayer( parameters, u"INPUT"_s, context );
  const QString name = parameterAsString( parameters, u"NAME"_s, context );

  if ( !layer )
    throw QgsProcessingException( QObject::tr( "Invalid input layer" ) );

  if ( name.isEmpty() )
    throw QgsProcessingException( QObject::tr( "Invalid (empty) layer name" ) );

  layer->setName( name );
  QgsProcessingContext::LayerDetails details( name, context.project(), name );
  details.forceName = true;
  context.addLayerToLoadOnCompletion( layer->id(), details );

  QVariantMap results;
  results.insert( u"OUTPUT"_s, layer->id() );
  return results;
}

///@endcond
